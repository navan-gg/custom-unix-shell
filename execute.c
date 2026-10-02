#include "shell.h"
#include "execute.h"
#include "builtins.h"
#include "signals.h"

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

// Applying the input and output redirections for a command
static int apply_redirection(Command *cmd) 
{
    if (cmd->infile != NULL) 
    {
        int fd = open(cmd->infile, O_RDONLY);
        if (fd < 0) 
        {
            perror(cmd->infile);
            return -1;
        }
        if (dup2(fd, STDIN_FILENO) < 0) 
        {
            perror("dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }
    if (cmd->outfile != NULL) 
    {
        int fd = open(cmd->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) 
        {
            perror(cmd->outfile);
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) 
        {
            perror("dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }
    return 0;
}

// command not found or execvp() failed for some reason
static void exec_failed(const char *name) 
{
    if (errno == ENOENT)
        fprintf(stderr, "%s: command not found\n", name);
    else
        fprintf(stderr, "%s: %s\n", name, strerror(errno));
    _exit(127);
}

// sets up the signal handlers and process group for a child process
static void child_setup(int background, const sigset_t *restore_mask) 
{
    sigprocmask(SIG_SETMASK, restore_mask, NULL);
    signal(SIGINT, background ? SIG_IGN : SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    if (background)
        setpgid(0, 0);
}

// runs a builtin command, handling any I/O redirection
static void run_builtin_redirected(Command *cmd) 
{
    if (cmd->infile == NULL && cmd->outfile == NULL) 
    {
        run_builtin(cmd);
        return;
    }

    int saved_in  = dup(STDIN_FILENO);
    int saved_out = dup(STDOUT_FILENO);

    if (apply_redirection(cmd) == 0)
        run_builtin(cmd);
    fflush(stdout);                 // flush into the file before restoring

    if (saved_in  != -1) { dup2(saved_in,  STDIN_FILENO);  close(saved_in);  }
    if (saved_out != -1) { dup2(saved_out, STDOUT_FILENO); close(saved_out); }
}

// Wait for all foreground processes to finish. called after fg_set.
static void wait_foreground(const sigset_t *prev)
{
    while (fg_pending() > 0)
        sigsuspend(prev);
    fg_clear(); //resets foreground processes when all children finish
}

// executes a single external command, handling I/O redirection and background execution
static void execute_external_single(Command *cmd, int background) 
{
    sigset_t block_chld, prev;
    sigemptyset(&block_chld);
    sigaddset(&block_chld, SIGCHLD);

    // blocking SIGCHLD
    sigprocmask(SIG_BLOCK, &block_chld, &prev);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        sigprocmask(SIG_SETMASK, &prev, NULL);
        return;
    }

    if (pid == 0) {                         // child
        child_setup(background, &prev);
        if (apply_redirection(cmd) != 0)
            _exit(1);
        execvp(cmd->argv[0], cmd->argv);
        exec_failed(cmd->argv[0]);
    }

    // parent
    if (background) 
    {
        printf("[background] started pid %d\n", pid);
        fflush(stdout);
        sigprocmask(SIG_SETMASK, &prev, NULL);  
    } else 
    {
        fg_set(&pid, 1);
        wait_foreground(&prev);
        sigprocmask(SIG_SETMASK, &prev, NULL);
    }
}


static void execute_pipeline(CommandLine *cl) 
{
    int   n = cl->ncmds;
    pid_t pids[MAX_CMDS];
    int   started = 0;
    int   prev_read = -1;                    

    sigset_t block_chld, prev;
    sigemptyset(&block_chld);
    sigaddset(&block_chld, SIGCHLD);
    sigprocmask(SIG_BLOCK, &block_chld, &prev);

    for (int i = 0; i < n; i++) {
        int pipefd[2] = { -1, -1 };
        if (i < n - 1) {
            if (pipe(pipefd) < 0) {
                perror("pipe");
                break;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            if (pipefd[0] != -1) close(pipefd[0]);
            if (pipefd[1] != -1) close(pipefd[1]);
            break;
        }

        if (pid == 0) {                     // child i
            child_setup(cl->background, &prev);

            // read from the previous stage
            if (prev_read != -1) {
                dup2(prev_read, STDIN_FILENO);
                close(prev_read);
            }
            // write to the next stage
            if (i < n - 1) {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }
            // a < or > wins over the pipe
            if (apply_redirection(&cl->cmds[i]) != 0)
                _exit(1);

            execvp(cl->cmds[i].argv[0], cl->cmds[i].argv);
            exec_failed(cl->cmds[i].argv[0]);
        }

        pids[started++] = pid;

        if (prev_read != -1)
            close(prev_read);
        if (i < n - 1) {
            close(pipefd[1]);
            prev_read = pipefd[0];
        }
    }
    if (prev_read != -1)
        close(prev_read);

    if (cl->background) 
    {
        if (started > 0) 
        {
            printf("[background] started pid %d\n", pids[started - 1]);
            fflush(stdout);
        }
        sigprocmask(SIG_SETMASK, &prev, NULL);
    } else {
        fg_set(pids, started);
        wait_foreground(&prev);
        sigprocmask(SIG_SETMASK, &prev, NULL);
    }
}


void execute_command_line(CommandLine *cl) {
    if (cl->ncmds == 0)
        return;

    if (cl->ncmds == 1) 
    {
        Command *cmd = &cl->cmds[0];
        
        if (is_builtin(cmd->argv[0])) 
        {
            run_builtin_redirected(cmd);
            return;
        }
        execute_external_single(cmd, cl->background);
    } else {
        execute_pipeline(cl);
    }
}

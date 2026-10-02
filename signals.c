#include "shell.h"
#include "signals.h"

#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_FG 512

// pids of the current foreground job. assuming that the pids become -1 when reaped
static volatile sig_atomic_t s_fg_pids[MAX_FG];
static volatile sig_atomic_t s_fg_n;         
static volatile sig_atomic_t s_fg_pending;   

void fg_set(const pid_t *pids, int n) 
{
    if (n > MAX_FG)
        n = MAX_FG;

    for (int i = 0; i < n; i++)
        s_fg_pids[i] = pids[i];

    s_fg_n = n;
    s_fg_pending = n;
}

int fg_pending(void) 
{
    return s_fg_pending;
}

void fg_clear(void) 
{
    s_fg_n = 0;
    s_fg_pending = 0;
}

// handles ctrl+c signal. just prints a new line and returns to the prompt
static void sigint_handler(int sig) 
{
    (void)sig;
    write(STDOUT_FILENO, "\n", 1);
}

// handles SIGCHLD signal. reaps all finished children and updates the foreground job tracking
static void sigchld_handler(int sig) 
{
    (void)sig;
    int saved_errno = errno;
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) 
    {
        for (int i = 0; i < s_fg_n; i++) 
        {
            if (s_fg_pids[i] == pid) 
            {
                s_fg_pids[i] = -1;
                if (s_fg_pending > 0)
                    s_fg_pending--; 
                break;
            }
        }
    }
    errno = saved_errno;
}

// sets up the signal handlers for SIGINT and SIGCHLD in main.c
void setup_signal_handlers(void) 
{
    struct sigaction sa_int, sa_chld;

    // for SIGINT
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;

    if (sigaction(SIGINT, &sa_int, NULL) != 0)
        perror("sigaction(SIGINT)");

    //for SIGCHLD
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa_chld, NULL) != 0)
        perror("sigaction(SIGCHLD)");
}

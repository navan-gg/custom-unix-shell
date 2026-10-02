#include "shell.h"
#include "builtins.h"
#include "history.h"

//pwd command
static int do_pwd(Command *cmd) 
{
    (void)cmd;
    char cwd[MAX_PATH_LEN];
    if (getcwd(cwd, sizeof(cwd)) == NULL) 
    {
        perror("pwd");
        return -1;
    }
    printf("%s\n", cwd);
    return 0;
}

//echo command
static int do_echo(Command *cmd) 
{
    for (int i = 1; i < cmd->argc; i++) 
    {
        if (i > 1)
            putchar(' ');
        fputs(cmd->argv[i], stdout);
    }
    putchar('\n');
    return 0;
}

//cd command
static int do_cd(Command *cmd) 
{
    char cwd[MAX_PATH_LEN];
    if (getcwd(cwd, sizeof(cwd)) == NULL) 
    {
        perror("cd: getcwd");
        return -1;
    }

    const char *target;
    int is_dash = 0;

    if (cmd->argc == 1) 
    {
        target = shell.home_dir;                    
    } else if (cmd->argc == 2) 
    {
        char *arg = cmd->argv[1];
        if (strcmp(arg, "~") == 0) 
        {
            target = shell.home_dir;                
        } else if (strcmp(arg, "-") == 0) 
        {         
            if (!shell.has_prev) 
            {
                fprintf(stderr, "No previous directory.\n");
                return -1;
            }
            target = shell.prev_dir;
            is_dash = 1;
        } else 
        {
            target = arg;                           
        }
    } else 
    {
        fprintf(stderr, "cd: too many arguments\n");
        return -1;
    }

    if (chdir(target) != 0) 
    {
        perror("cd");
        return -1;
    }

    // Update the previous directory and set has_prev to true
    strncpy(shell.prev_dir, cwd, sizeof(shell.prev_dir) - 1);
    shell.prev_dir[sizeof(shell.prev_dir) - 1] = '\0';
    shell.has_prev = 1;

    if (is_dash) {
        char newcwd[MAX_PATH_LEN];
        if (getcwd(newcwd, sizeof(newcwd)) != NULL)
            printf("%s\n", newcwd);
    }
    return 0;
}

// history command
static int do_history(Command *cmd) 
{
    (void)cmd;
    history_show();
    return 0;
}

// check if a command is a built-in
int is_builtin(const char *name) 
{
    return strcmp(name, "cd") == 0 || strcmp(name, "pwd") == 0 || strcmp(name, "echo") == 0 || strcmp(name, "history") == 0;
}

int run_builtin(Command *cmd) 
{
    const char *name = cmd->argv[0];
    if (strcmp(name, "cd") == 0)      return do_cd(cmd);
    if (strcmp(name, "pwd") == 0)     return do_pwd(cmd);
    if (strcmp(name, "echo") == 0)    return do_echo(cmd);
    if (strcmp(name, "history") == 0) return do_history(cmd);
    return -1;                          // won't happen if is_builtin was checked
}

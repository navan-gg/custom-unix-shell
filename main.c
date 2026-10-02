#include "shell.h"
#include "display.h"
#include "parser.h"
#include "execute.h"
#include "history.h"
#include "signals.h"

#include <errno.h>

ShellState shell; //global variable to hold shell state


static void init_shell(void) 
{
    if (getcwd(shell.home_dir, sizeof(shell.home_dir)) == NULL) 
    {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
    shell.has_prev = 0;
    shell.prev_dir[0] = '\0';
}

int main() 
{
    char line[MAX_INPUT];

    init_shell();
    history_load();
    setup_signal_handlers();

    while (1) 
    {
        print_prompt();

        errno = 0;
        if (fgets(line, sizeof(line), stdin) == NULL) 
        {
            if (errno == EINTR) 
            {
                // Ctrl+C interrupted
                clearerr(stdin);
                continue;
            }
            
            // Ctrl+D or EOF 
            if (isatty(STDIN_FILENO))
                printf("\n");
            break;
        }

        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        if (line[0] == '\0')
            continue;

        history_add(line);

        CommandLine cl;
        if (parse_line(line, &cl) == 0)
            execute_command_line(&cl);
        free_command_line(&cl);
    }

    return 0;
}

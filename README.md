For the full problem statement and requirements of this project, refer to the project specification document: [OS_Project_Specification.pdf](OS_Project_Specification.pdf)

------------------------------------------------------------------------------------------------------------

How to build and run the project:

1) make          -> compiles the codebase and produces ./shell
2) make run      -> compiles and executes ./shell
3) make debug    -> compile and launch under the debugger
4) make clean    -> removes all object and executable files
One can use make run to directly execute the shell or make all followed by "./shell" to execute the shell.

------------------------------------------------------------------------------------------------------------
Files:

The code is split up by feature for modularity.

- shell.h      -> constants and the ShellState struct (home dir, previous dir)
- main.c       -> the main loop: print prompt, read a line, parse it, run it
- display.c    -> builds and prints the prompt
- parser.c     -> breaks the input line into tokens and command structs
- builtins.c   -> cd, pwd, and echo implementation
- history.c    -> keeps the command history and saves it to a file
- execute.c    -> fork/exec, plus background, redirection and pipes
- signals.c    -> the Ctrl+C and Ctrl+D handler

Each .c has a matching .h with just its function declarations and some important structs used for it

------------------------------------------------------------------------------------------------------------

Assumptions:

- "Home" means wherever the shell was started, not $HOME as the specifications ask for.

- The history file is .shell_history in that home directory. Every non-empty line is saved (including    built-ins and history itself). The oldest is at the top and the newest at the bottom

- A quoted operator like "|" is just text, not a pipe.

- Background jobs are put in their own process group so Ctrl+C at the terminal doesn't reach them.

- An unknown command prints "<name>: command not found". Other system call failures are reported with perror.

- There are some fixed size limits: an input line and a path are capped at 4096 characters, a line can have up to 1024 tokens/arguments and up to 256 piped commands. The max input size can only be 4096 characters.

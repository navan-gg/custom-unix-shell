#ifndef PARSER_H
#define PARSER_H

#include "shell.h"

#define MAX_TOKENS 1024     
#define MAX_ARGS   1024     
#define MAX_CMDS   256      

// Struct for command: argv, argc, and optional I/O redirection files
typedef struct {
    char *argv[MAX_ARGS];
    int   argc;
    char *infile;           // only for I/O redirection
    char *outfile;          // only for I/O redirection
} Command;

//Struct for command line
typedef struct {
    Command cmds[MAX_CMDS];
    int     ncmds;
    int     background;
    char   *tokens[MAX_TOKENS]; // array of strings for tokens
    char    tok_is_op[MAX_TOKENS];   // to track which tokens are operators
    int     ntokens;
} CommandLine;

int  parse_line(const char *line, CommandLine *cl);

void free_command_line(CommandLine *cl);

#endif 

#ifndef SHELL_H
#define SHELL_H

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT     4096      
#define MAX_PATH_LEN  4096      
#define HISTORY_SIZE  20        
#define HISTORY_SHOW  10        
#define HISTORY_FILE  ".shell_history"

//Shell state structure
typedef struct {
    char home_dir[MAX_PATH_LEN];
    char prev_dir[MAX_PATH_LEN];    
    int  has_prev;                  //check for atleast one cd command has been executed or not
} ShellState;

extern ShellState shell;            //declares shell as a global variable. actually defined in main.c

#endif 

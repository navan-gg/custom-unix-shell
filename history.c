#include "shell.h"
#include "history.h"

#include <ctype.h>

//here, assumed that the history is stored with oldest first and newest last
static char hist[HISTORY_SIZE][MAX_INPUT];
static int  hist_count;      

// this generates the path to the history file 
static void history_path(char *buf, size_t n) 
{
    snprintf(buf, n, "%s/%s", shell.home_dir, HISTORY_FILE);
}


//copies line into out without leading/trailing whitespace, so "ls" and "  ls " count as the same command
static void trim_copy(char *out, const char *line)
{
    while (isspace((unsigned char)*line))
        line++;

    size_t len = strlen(line);
    while (len > 0 && isspace((unsigned char)line[len - 1]))
        len--;
    if (len > MAX_INPUT - 1)
        len = MAX_INPUT - 1;

    memcpy(out, line, len);
    out[len] = '\0';
}

static void push_entry(const char *line)
{
    if (hist_count < HISTORY_SIZE) 
    {
        strncpy(hist[hist_count], line, MAX_INPUT - 1);
        hist[hist_count][MAX_INPUT - 1] = '\0';
        hist_count++;
    } else 
    {
        memmove(hist[0], hist[1], (HISTORY_SIZE - 1) * MAX_INPUT);
        strncpy(hist[HISTORY_SIZE - 1], line, MAX_INPUT - 1);
        hist[HISTORY_SIZE - 1][MAX_INPUT - 1] = '\0';
    }
}

//this writes the whole buffer back to the history file
static void history_save(void) 
{
    char path[MAX_PATH_LEN];
    history_path(path, sizeof(path));

    FILE *f = fopen(path, "w");
    if (f == NULL) {
        perror("history: fopen");
        return;
    }
    for (int i = 0; i < hist_count; i++)
        fprintf(f, "%s\n", hist[i]);
    fclose(f);
}

//used in main.c to load the history from the file into the buffer
void history_load(void) 
{
    hist_count = 0;

    char path[MAX_PATH_LEN];
    history_path(path, sizeof(path));

    FILE *f = fopen(path, "r");
    if (f == NULL)
        return;                     

    char line[MAX_INPUT];
    char trimmed[MAX_INPUT];

    while (fgets(line, sizeof(line), f) != NULL)
    {
        trim_copy(trimmed, line); //also strips the trailing newline
        if (trimmed[0] == '\0') //skip empty lines
            continue;
        push_entry(trimmed);
    }
    fclose(f);
}

void history_add(const char *line) 
{
    char trimmed[MAX_INPUT];
    trim_copy(trimmed, line);

    if (trimmed[0] == '\0')
        return;
    //duplicate check. if duplicate, then don't add to history
    if (hist_count > 0 && strcmp(hist[hist_count - 1], trimmed) == 0)
        return;

    push_entry(trimmed);
    history_save();
}

void history_show(void) 
{
    int start = (hist_count > HISTORY_SHOW) ? hist_count - HISTORY_SHOW : 0;
    for (int i = start; i < hist_count; i++)
    {
        printf("%s\n", hist[i]);
    }
}

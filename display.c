#include "shell.h"
#include "display.h"

#include <pwd.h>
#include <sys/types.h>

// convert cwd to something we can print
static void to_display_path(const char *cwd, char *out, size_t out_size) 
{
    size_t home_len = strlen(shell.home_dir);

    if (strcmp(cwd, shell.home_dir) == 0) // adds ~ for home directory
    {
        snprintf(out, out_size, "~");
    } 
    else if (strncmp(cwd, shell.home_dir, home_len) == 0 && cwd[home_len] == '/')// adds ~ for subdirectories of home directory
    {
        snprintf(out, out_size, "~%s", cwd + home_len);
    } 
    else // for other directories
    {
        snprintf(out, out_size, "%s", cwd);
    }
}

void print_prompt(void) 
{
    char username[256];
    char hostname[256];
    char cwd[MAX_PATH_LEN];
    char display[MAX_PATH_LEN + 2];

    
    struct passwd *pw = getpwuid(getuid());
    if (pw != NULL)
        snprintf(username, sizeof(username), "%s", pw->pw_name);
    else
        snprintf(username, sizeof(username), "unknown");

    if (gethostname(hostname, sizeof(hostname)) != 0)
        snprintf(hostname, sizeof(hostname), "unknown");

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        snprintf(cwd, sizeof(cwd), "?");
    }

    to_display_path(cwd, display, sizeof(display));

    printf("<%s@%s:%s> ", username, hostname, display);
    fflush(stdout);
}

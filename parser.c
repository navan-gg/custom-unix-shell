#include "parser.h"

// True if token i is a real operator with the given char.
static int is_op(const CommandLine *cl, int i, char c) 
{
    return cl->tok_is_op[i] && cl->tokens[i][0] == c;
}

// True if token i is any real operator.
static int is_any_op(const CommandLine *cl, int i) 
{
    return cl->tok_is_op[i];
}

// Add one token to cl, recording whether it's an operator.
static void add_token(CommandLine *cl, const char *s, int op) 
{
    cl->tok_is_op[cl->ntokens] = (char)op;
    cl->tokens[cl->ntokens] = strdup(s);
    cl->ntokens++;
}

// main tokenizer function
static int tokenize(const char *line, CommandLine *cl) 
{
    const char *p = line;
    char buf[MAX_INPUT];

    while (*p) 
    {
        while (*p == ' ' || *p == '\t')
            p++;
        if (*p == '\0')
            break;

        if (cl->ntokens >= MAX_TOKENS) 
        {
            fprintf(stderr, "shell: too many tokens\n");
            return -1;
        }

        if (*p == '|' || *p == '<' || *p == '>' || *p == '&') 
        {
            buf[0] = *p;
            buf[1] = '\0';
            add_token(cl, buf, 1);
            p++;
            continue;
        }

        // copying quoted parts through 
        int bi = 0;
        while (*p && *p != ' ' && *p != '\t' && *p != '|' && *p != '<' && *p != '>' && *p != '&') 
        {
            if (*p == '"' || *p == '\'') 
            {
                char quote = *p++;
                while (*p && *p != quote) 
                {
                    if (bi < MAX_INPUT - 1)
                        buf[bi++] = *p;
                    p++;
                }
                if (*p != quote) 
                {
                    fprintf(stderr, "shell: syntax error: unterminated quote\n");
                    return -1;
                }
                p++;
            } else 
            {
                if (bi < MAX_INPUT - 1)
                    buf[bi++] = *p;
                p++;
            }
        }
        buf[bi] = '\0';
        add_token(cl, buf, 0);
    }
    return 0;
}

int parse_line(const char *line, CommandLine *cl) //caled in main loop
{
    memset(cl, 0, sizeof(*cl));

    if (tokenize(line, cl) < 0)
        return -1;
    if (cl->ntokens == 0)
        return 0;                           

    
    if (is_op(cl, cl->ntokens - 1, '&')) {
        cl->background = 1;
        free(cl->tokens[cl->ntokens - 1]);
        cl->tokens[cl->ntokens - 1] = NULL;
        cl->ntokens--;
        if (cl->ntokens == 0) {
            fprintf(stderr, "shell: syntax error near '&'\n");
            return -1;
        }
    }
    // & end  check
    for (int i = 0; i < cl->ntokens; i++) {
        if (is_op(cl, i, '&')) {
            fprintf(stderr, "shell: syntax error: '&' is only allowed at the very end\n");
            return -1;
        }
    }

    // split on |, pick up < and > redirections.
    Command *cur = &cl->cmds[0];
    cl->ncmds = 1;

    int i = 0;
    while (i < cl->ntokens) 
    {
        char *tok = cl->tokens[i];

        if (is_op(cl, i, '|')) {
            if (cur->argc == 0) {
                fprintf(stderr, "shell: syntax error near '|'\n");
                return -1;
            }
            if (cl->ncmds >= MAX_CMDS) {
                fprintf(stderr, "shell: too many pipe segments\n");
                return -1;
            }
            cur->argv[cur->argc] = NULL;
            cur = &cl->cmds[cl->ncmds++];
            i++;
        } else if (is_op(cl, i, '<')) {
            if (i + 1 >= cl->ntokens || is_any_op(cl, i + 1)) 
            {
                fprintf(stderr, "shell: syntax error: expected filename after '<'\n");
                return -1;
            }
            cur->infile = cl->tokens[i + 1];
            i += 2;
        } else if (is_op(cl, i, '>')) {
            if (i + 1 >= cl->ntokens || is_any_op(cl, i + 1)) 
            {
                fprintf(stderr, "shell: syntax error: expected filename after '>'\n");
                return -1;
            }
            cur->outfile = cl->tokens[i + 1];
            i += 2;
        } else {
            if (cur->argc >= MAX_ARGS - 1) 
            {
                fprintf(stderr, "shell: too many arguments\n");
                return -1;
            }
            cur->argv[cur->argc++] = tok;
            i++;
        }
    }

    if (cur->argc == 0) {                    // trailing | with nothing after it
        fprintf(stderr, "shell: syntax error: expected command\n");
        return -1;
    }
    cur->argv[cur->argc] = NULL;
    return 0;
}

void free_command_line(CommandLine *cl) 
{
    for (int i = 0; i < cl->ntokens; i++) 
    {
        free(cl->tokens[i]);
        cl->tokens[i] = NULL;
    }
    cl->ntokens = 0;
    cl->ncmds = 0;
}

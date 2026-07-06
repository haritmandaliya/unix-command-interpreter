/******************************************************************************
 * File        : parser.c
 * Description : Converts tokens into Command structure and executes
 *               sequential commands separated by ';' and pipelines '|'
 ******************************************************************************/

#include "../include/parser.h"
#include "../include/execute.h"
#include "../include/builtin.h"

/******************************************************************************
 * build_command()
 *
 * Converts a specific sub-range of tokens into a single Command structure.
 * Extract arguments, input files (`<`), and output files (`>`).
 *
 * Parameters:
 *   - tokens: The global token stream.
 *   - start: Starting index of the token range for this command.
 *   - end: Ending index of the token range (inclusive).
 *   - cmd: Pointer to the Command struct to populate.
 *
 * Returns:
 *   - The count of arguments (argc) in the constructed command.
 ******************************************************************************/
int build_command(Token tokens[], int start, int end, Command *cmd)
{
    int i;

    /* Initialize command variables to defaults */
    cmd->argc = 0;
    cmd->input_file = NULL;
    cmd->output_file = NULL;

    /* Loop through the token range assigned to this command */
    for (i = start; i <= end; i++)
    {
        switch (tokens[i].type)
        {
            case TOKEN_WORD:
                /* 
                 * A normal word is an argument or the command name itself.
                 * Add it to the argv array and increment argc counter.
                 */
                cmd->argv[cmd->argc++] = tokens[i].text;
                break;

            case TOKEN_INPUT:
                /* 
                 * Input redirection '<' must specify a source file.
                 * The filename should be the token immediately following '<'.
                 */
                if (i + 1 <= end)
                {
                    cmd->input_file = tokens[i + 1].text; /* Point to the filename */
                    i++; /* Skip filename token in the next iteration */
                }
                break;

            case TOKEN_OUTPUT:
                /* 
                 * Output redirection '>' must specify a target file.
                 * The filename should be the token immediately following '>'.
                 */
                if (i + 1 <= end)
                {
                    cmd->output_file = tokens[i + 1].text; /* Point to the filename */
                    i++; /* Skip filename token in the next iteration */
                }
                break;

            default:
                /* Semicolons and Pipes are separators, not command elements */
                break;
        }
    }

    /* 
     * In standard C, the argument vector (argv) MUST be terminated by a NULL pointer.
     * This signals execvp() where the argument list ends.
     */
    cmd->argv[cmd->argc] = NULL;

    /* Return total arguments parsed */
    return cmd->argc;
}

/******************************************************************************
 * validate_tokens()
 *
 * Checks for shell syntax errors in the token stream before any execution.
 * Prints descriptive errors to stderr and returns status.
 *
 * Parameters:
 *   - tokens: The global token stream.
 *   - total: Total token count.
 *
 * Returns:
 *   - 1 if the token stream is valid.
 *   - 0 if a syntax error is detected (prints error).
 ******************************************************************************/
int validate_tokens(Token tokens[], int total)
{
    /* If there are no tokens, it is technically valid (empty input) */
    if (total == 0)
    {
        return 1;
    }

    /* 
     * A semicolon at the very beginning is a syntax error:
     * e.g. `; ls`
     */
    if (tokens[0].type == TOKEN_SEMICOLON)
    {
        fprintf(stderr, "syntax error near unexpected token ';'\n");
        return 0;
    }

    /* 
     * A pipe at the very beginning is a syntax error:
     * e.g. `| ls`
     */
    if (tokens[0].type == TOKEN_PIPE)
    {
        fprintf(stderr, "syntax error near unexpected token '|'\n");
        return 0;
    }

    /* Scan the token stream looking for sequential or placement anomalies */
    for (int i = 0; i < total; i++)
    {
        if (tokens[i].type == TOKEN_PIPE)
        {
            if (i + 1 < total)
            {
                /* 
                 * Two consecutive pipe tokens mean '||':
                 * e.g. `ls || wc`
                 */
                if (tokens[i + 1].type == TOKEN_PIPE)
                {
                    fprintf(stderr, "syntax error near unexpected token '||'\n");
                    return 0;
                }
                /* 
                 * A pipe followed by a semicolon is invalid:
                 * e.g. `ls | ;`
                 */
                if (tokens[i + 1].type == TOKEN_SEMICOLON)
                {
                    fprintf(stderr, "syntax error near unexpected token ';'\n");
                    return 0;
                }
            }
            else
            {
                /* 
                 * A pipe at the very end of the command is invalid:
                 * e.g. `ls |`
                 */
                fprintf(stderr, "syntax error: unexpected end of file after '|'\n");
                return 0;
            }
        }
        else if (tokens[i].type == TOKEN_SEMICOLON)
        {
            if (i + 1 < total)
            {
                /* 
                 * Two consecutive semicolons mean ';;':
                 * e.g. `ls ;;`
                 */
                if (tokens[i + 1].type == TOKEN_SEMICOLON)
                {
                    fprintf(stderr, "syntax error near unexpected token ';;'\n");
                    return 0;
                }
                /* 
                 * A semicolon followed by a pipe is invalid:
                 * e.g. `ls ; | wc`
                 */
                if (tokens[i + 1].type == TOKEN_PIPE)
                {
                    fprintf(stderr, "syntax error near unexpected token '|'\n");
                    return 0;
                }
            }
        }
        else if (tokens[i].type == TOKEN_INPUT || tokens[i].type == TOKEN_OUTPUT)
        {
            if (i + 1 < total)
            {
                /* 
                 * Redirection (< or >) must be followed by a WORD (filename).
                 * Redirection followed by a pipe or semicolon is a syntax error.
                 */
                if (tokens[i + 1].type != TOKEN_WORD)
                {
                    fprintf(stderr, "syntax error near unexpected token '%s'\n", tokens[i + 1].text);
                    return 0;
                }
            }
            else
            {
                /* 
                 * Redirection at the end of the line is missing a filename:
                 * e.g. `ls >` or `grep <`
                 */
                fprintf(stderr, "syntax error: missing filename after '%s'\n", tokens[i].text);
                return 0;
            }
        }
    }

    /* All checks passed! Stream is valid */
    return 1;
}

/******************************************************************************
 * is_state_modifying_builtin()
 *
 * Check if the command is a built-in command that modifies the parent shell's
 * environment ('cd' changes directory, 'quit' exits the process).
 *
 * Parameters:
 *   - cmd: Pointer to the Command structure.
 *
 * Returns:
 *   - 1 if it is a state-modifying built-in, 0 otherwise.
 ******************************************************************************/
static int is_state_modifying_builtin(Command *cmd)
{
    if (cmd->argc > 0)
    {
        /* 
         * 'cd' and 'quit' modify shell states, which must happen in the parent process.
         * Running them in a subshell pipeline renders them ineffective.
         */
        if (strcmp(cmd->argv[0], "cd") == 0 || strcmp(cmd->argv[0], "quit") == 0)
        {
            return 1;
        }
    }
    return 0;
}

/******************************************************************************
 * execute_pipeline()
 *
 * Executes a pipeline segment of commands. Works for both single commands
 * (num_cmds = 1) and complex multi-piped commands (e.g. `cat a | grep b | sort`).
 *
 * Parameters:
 *   - tokens: The global token stream.
 *   - start: Start index of the pipeline segment.
 *   - end: End index of the pipeline segment.
 * ****************************************************************************/
void execute_pipeline(Token tokens[], int start, int end)
{
    /* Arrays to store boundaries of each command within the token range */
    int cmd_start[SHELL_MAX_TOKENS];
    int cmd_end[SHELL_MAX_TOKENS];
    int num_cmds = 0; /* Number of commands in this pipeline */
    int current_start = start;

    /* Scan the range to find pipes '|' and segment commands */
    for (int i = start; i <= end; i++)
    {
        if (tokens[i].type == TOKEN_PIPE)
        {
            cmd_start[num_cmds] = current_start; /* Start index of current command */
            cmd_end[num_cmds] = i - 1;           /* End index of current command */
            num_cmds++;                          /* Increment command counter */
            current_start = i + 1;               /* Next command starts after the pipe */
        }
    }
    /* Capture the boundaries of the final command in the pipeline */
    cmd_start[num_cmds] = current_start;
    cmd_end[num_cmds] = end;
    num_cmds++;

    /* 
     * Optimization / Precedence Rule:
     * If there is only 1 command (no pipes), execute it directly in the parent process
     * (or let it fork in execute_command if it is an external command).
     */
    if (num_cmds == 1)
    {
        Command cmd;
        /* Build command structure and execute it */
        if (build_command(tokens, cmd_start[0], cmd_end[0], &cmd) > 0)
        {
            execute_command(&cmd);
        }
        return;
    }

    /* Array of Command structs to hold parsed data for each command in the pipeline */
    Command cmds[SHELL_MAX_TOKENS];

    /* Parse all commands in the pipeline and check for pipeline built-in errors */
    for (int i = 0; i < num_cmds; i++)
    {
        build_command(tokens, cmd_start[i], cmd_end[i], &cmds[i]);
        
        /* 
         * State-modifying built-ins like 'cd' and 'quit' cannot run in a pipeline.
         * For example, in `cd .. | pwd`, the directory shift in `cd` would happen
         * in a forked child and immediately vanish when the child terminates.
         */
        if (is_state_modifying_builtin(&cmds[i]))
        {
            fprintf(stderr, "Error: %s cannot be executed in a pipeline\n", cmds[i].argv[0]);
            return;
        }
    }

    /* 
     * Pipeline Setup:
     * An N-stage pipeline requires N-1 pipe file descriptor pairs.
     */
    int num_pipes = num_cmds - 1;
    
    /* Dynamically allocate an array to hold file descriptors for all pipes */
    int *pipes_fds = malloc(2 * num_pipes * sizeof(int));
    if (pipes_fds == NULL)
    {
        perror("malloc");
        return;
    }

    /* 
     * Initialize all pipes.
     * Each pipe() call creates two file descriptors:
     * pipes_fds[i*2] is the read end of pipe i,
     * pipes_fds[i*2 + 1] is the write end of pipe i.
     */
    for (int i = 0; i < num_pipes; i++)
    {
        if (pipe(pipes_fds + i * 2) < 0)
        {
            perror("pipe");
            /* Close all pipes that were already successfully created before failure */
            for (int j = 0; j < i * 2; j++)
            {
                close(pipes_fds[j]);
            }
            free(pipes_fds);
            return;
        }
    }

    /* Array to keep track of Process IDs (PIDs) of all spawned child processes */
    pid_t *pids = malloc(num_cmds * sizeof(pid_t));
    if (pids == NULL)
    {
        perror("malloc");
        /* Close all pipe file descriptors and free memory */
        for (int j = 0; j < 2 * num_pipes; j++)
        {
            close(pipes_fds[j]);
        }
        free(pipes_fds);
        return;
    }

    /* Fork and launch each process in the pipeline */
    for (int i = 0; i < num_cmds; i++)
    {
        pids[i] = fork(); /* Create child process */

        if (pids[i] < 0)
        {
            perror("fork");
            /* 
             * Fork failure handler:
             * Close all pipes, wait for any children already spawned, free memory, and return.
             */
            for (int j = 0; j < 2 * num_pipes; j++)
            {
                close(pipes_fds[j]);
            }
            for (int j = 0; j < i; j++)
            {
                waitpid(pids[j], NULL, 0);
            }
            free(pids);
            free(pipes_fds);
            return;
        }

        /*======================================================================
         * Child Process Code (runs in the context of the spawned subprocess)
         *======================================================================*/
        if (pids[i] == 0)
        {
            /* 
             * 1. Bind stdin to the previous command's pipe output.
             * If this is not the first command (i > 0), redirect standard input
             * to read from the read-end of the previous pipe: pipes_fds[(i - 1) * 2]
             */
            if (i > 0)
            {
                if (dup2(pipes_fds[(i - 1) * 2], STDIN_FILENO) < 0)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            /* 
             * 2. Bind stdout to the next command's pipe input.
             * If this is not the last command (i < num_cmds - 1), redirect standard output
             * to write to the write-end of the current pipe: pipes_fds[i * 2 + 1]
             */
            if (i < num_cmds - 1)
            {
                if (dup2(pipes_fds[i * 2 + 1], STDOUT_FILENO) < 0)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            /* 
             * 3. Close all pipe copies in the child process.
             * The children duplicate the descriptors they need (using dup2),
             * and must close the extra pipe copies to avoid descriptor leaks and EOF hangs.
             */
            for (int j = 0; j < 2 * num_pipes; j++)
            {
                close(pipes_fds[j]);
            }

            /* 
             * 4. Handle Input File Redirection (<):
             * If an input file is specified (e.g. `sort < input.txt`), open the file
             * in read-only mode and bind STDIN_FILENO to it (overriding the pipe input).
             */
            if (cmds[i].input_file != NULL)
            {
                int fd = open(cmds[i].input_file, O_RDONLY);
                if (fd < 0)
                {
                    perror(cmds[i].input_file);
                    exit(EXIT_FAILURE);
                }
                if (dup2(fd, STDIN_FILENO) < 0)
                {
                    perror("dup2");
                    close(fd);
                    exit(EXIT_FAILURE);
                }
                close(fd); /* Close original descriptor after duplicating */
            }

            /* 
             * 5. Handle Output File Redirection (>):
             * If an output file is specified (e.g. `ls > out.txt`), open the file
             * in write-only, create, truncate mode (0644 permissions) and bind
             * STDOUT_FILENO to it (overriding the pipe output).
             */
            if (cmds[i].output_file != NULL)
            {
                int fd = open(cmds[i].output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0)
                {
                    perror(cmds[i].output_file);
                    exit(EXIT_FAILURE);
                }
                if (dup2(fd, STDOUT_FILENO) < 0)
                {
                    perror("dup2");
                    close(fd);
                    exit(EXIT_FAILURE);
                }
                close(fd); /* Close original descriptor after duplicating */
            }

            /* 
             * 6. Execute Built-in or External Command:
             * Non-state-modifying builtins like 'hist', 'curPid', 'pPid' can execute
             * in the child process. If execute_builtin returns 1, exit with success.
             */
            if (execute_builtin(&cmds[i]))
            {
                exit(EXIT_SUCCESS);
            }

            /* Otherwise, replace child's address space with the external command */
            execvp(cmds[i].argv[0], cmds[i].argv);
            
            /* If execvp returns, it means it failed. Print error and exit */
            perror(cmds[i].argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    /*======================================================================
     * Parent Process Code (continues execution in the parent shell)
     *======================================================================*/

    /* 
     * Close all pipe file descriptors in the parent shell.
     * If the parent doesn't close the write ends of the pipes, readers downstream
     * will never detect End-of-File (EOF) and will hang waiting for more input.
     */
    for (int i = 0; i < 2 * num_pipes; i++)
    {
        close(pipes_fds[i]);
    }

    /* 
     * Wait for all spawned child processes to finish.
     * This avoids creating 'zombie' processes (terminated processes whose
     * status has not been retrieved by the parent).
     */
    for (int i = 0; i < num_cmds; i++)
    {
        waitpid(pids[i], NULL, 0);
    }

    /* Clean up allocated dynamic arrays to avoid memory leaks */
    free(pids);
    free(pipes_fds);
}

/******************************************************************************
 * parse_tokens()
 *
 * Scans the token stream for Semicolons ';'.
 * Splits the stream into sequential command segments and runs them one by one.
 *
 * Parameters:
 *   - tokens: The global token stream.
 *   - total: Total token count.
 ******************************************************************************/
void parse_tokens(Token tokens[], int total)
{
    /* First, check the entire input line for syntax errors */
    if (!validate_tokens(tokens, total))
    {
        return; /* Stop parsing if syntax is invalid */
    }

    int start = 0;
    int end;

    /* Loop through the token stream */
    for (end = 0; end < total; end++)
    {
        /* Semicolon ';' separates sequential commands */
        if (tokens[end].type == TOKEN_SEMICOLON)
        {
            /* If there is a command segment between start and semicolon */
            if (start < end)
            {
                /* Run the segment (which could be a pipeline) */
                execute_pipeline(tokens, start, end - 1);
            }

            /* The next command starts after the semicolon */
            start = end + 1;
        }
    }

    /* Execute the final command/pipeline segment (if there is one) */
    if (start < total)
    {
        execute_pipeline(tokens, start, total - 1);
    }
}

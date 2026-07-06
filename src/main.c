/******************************************************************************
 * File Name   : main.c
 * Project     : UNIX Command Interpreter
 * Description : Main entry point of the shell.
 ******************************************************************************/

/* 
 * We define _DEFAULT_SOURCE to expose modern POSIX functions like gethostname()
 * which are not exposed by default under the strict C11 standard (-std=c11).
 */
#define _DEFAULT_SOURCE

/* Standard I/O functions like printf, fgets, and perror */
#include "../include/shell.h"
/* Prototypes for command parsing and validation */
#include "../include/parser.h"
/* Prototypes for built-in shell commands (cd, quit, etc.) */
#include "../include/builtin.h"
/* Prototypes for command history tracking */
#include "../include/history.h"
/* Prototypes for command execution (fork, execvp) */
#include "../include/execute.h"
/* Prototypes for lexical analysis (tokenizer) */
#include "../include/tokenizer.h"
/* Structures for shell Command definitions */
#include "../include/command.h"

/* 
 * Global counter to track the current command number.
 * Exposed to other files via extern declaration in shell.h.
 */
int command_number = 1;

/******************************************************************************
 * Main Function
 * Entry point of our command interpreter. Handles both interactive shell loop
 * and non-interactive command string execution (-c option).
 ******************************************************************************/
int main(int argc, char *argv[])
{
    /* Buffer to store the current machine's hostname (e.g. "harit") */
    char hostname[SHELL_HOSTNAME_SIZE];

    /* 
     * Get the machine's hostname.
     * gethostname() fills the 'hostname' buffer with the name of the local host.
     * Returns 0 on success. If it fails, fallback to "unknown".
     */
    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        strcpy(hostname, "unknown");
    }

    /* 
     * Check if arguments were passed to the shell.
     * If there is more than 1 argument (argc > 1), check for non-interactive mode.
     */
    if (argc > 1)
    {
        /* 
         * Support the -c option: ./prog01 -c "command_string"
         * We expect exactly 3 arguments (argv[0] is binary, argv[1] is "-c", argv[2] is command).
         */
        if (argc == 3 && strcmp(argv[1], "-c") == 0)
        {
            /* Input buffer to hold a copy of the command string to be tokenized */
            char input[SHELL_MAX_INPUT];
            
            /* Safe string copy to prevent buffer overflow */
            strncpy(input, argv[2], sizeof(input) - 1);
            input[sizeof(input) - 1] = '\0'; /* Ensure null-termination */

            /* Array to hold token structures produced by the tokenizer */
            Token tokens[SHELL_MAX_TOKENS];
            
            /* Tokenize the input string and get total token count */
            int total = tokenize(input, tokens);
            
            /* Parse and execute the tokens */
            parse_tokens(tokens, total);
            
            /* Exit the shell after executing the non-interactive command */
            return 0;
        }
        else
        {
            /* If arguments are invalid, print correct usage details to stderr */
            fprintf(stderr, "Usage: %s [-c command]\n", argv[0]);
            return 1;
        }
    }

    /* Input buffer to store user command lines in interactive mode */
    char input[SHELL_MAX_INPUT];

    /* Copy of the original command line to store in history (since tokenization modifies input) */
    char original_input[SHELL_MAX_INPUT];

    /* 
     * Infinite loop for the interactive shell session.
     * Runs until EOF (Ctrl-D) or the built-in "quit" command is executed.
     */
    while (1)
    {
        /* 
         * Print the shell prompt, e.g. "<1 harit> "
         * Displaying the current command number and machine's hostname.
         */
        printf("<%d %s> ", command_number, hostname);
        
        /* 
         * Flush standard output.
         * Since the prompt doesn't end with a newline '\n', stdout is buffered.
         * fflush(stdout) forces the prompt to print to the screen immediately.
         */
        fflush(stdout);

        /* 
         * Read one line of text from standard input (stdin).
         * fgets reads up to sizeof(input) - 1 characters.
         * Returns NULL on EOF (e.g. user presses Ctrl-D).
         */
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("\n"); /* Print a clean newline on shell exit */
            break;        /* Break out of the loop and terminate */
        }

        /* 
         * Remove the trailing newline character '\n' appended by fgets.
         * strcspn returns the length of the string segment before the first '\n'.
         * We set that character to '\0' to strip the newline.
         */
        input[strcspn(input, "\n")] = '\0';

        /* 
         * If the user just pressed Enter (empty command),
         * skip the rest of the loop and show the prompt again.
         */
        if (strlen(input) == 0)
        {
            continue;
        }

        /* 
         * Save a copy of the input string.
         * Some downstream parsing functions may modify strings in-place.
         */
        strcpy(original_input, input);

        /* 
         * Store the user's entered command in the history array.
         * Enables command tracking for the built-in 'hist' command.
         */
        add_history(original_input);

        /* Array of token structures to be populated by the tokenizer */
        Token tokens[SHELL_MAX_TOKENS];
        
        /* Tokenize the input string and get total token count */
        int total = tokenize(input, tokens);
        
        /* Parse and execute the command(s) or pipelines represented by the tokens */
        parse_tokens(tokens, total);

        /* 
         * Increment the command counter for the next prompt.
         * Only increments after a command attempt is processed.
         */
        command_number++;
    }

    return 0;
}

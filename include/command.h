#ifndef COMMAND_H
#define COMMAND_H

#include "shell.h"

/*=============================================================================
    Command Structure:
    Represents a parsed single executable command.
    Example: `ls -la > output.txt`
      - argv = ["ls", "-la", NULL]
      - argc = 2
      - input_file = NULL
      - output_file = "output.txt"
=============================================================================*/
typedef struct
{
    /* Array of argument strings passed to the command (argv[0] is the command name) */
    char *argv[SHELL_MAX_ARGS];

    /* Number of arguments in the command */
    int argc;

    /* Path of file for input redirection (<). NULL if stdin is not redirected */
    char *input_file;

    /* Path of file for output redirection (>). NULL if stdout is not redirected */
    char *output_file;

} Command;

#endif

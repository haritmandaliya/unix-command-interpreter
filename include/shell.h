#ifndef SHELL_H
#define SHELL_H

/*=============================================================================
    Standard Library Headers
    Include standard libraries for systems programming, file control, and memory.
=============================================================================*/

#include <stdio.h>    /* standard I/O (printf, fprintf, perror, fgets) */
#include <stdlib.h>   /* standard library utility functions (malloc, free, exit) */
#include <string.h>   /* string operations (strcmp, strcpy, strcspn, strncpy) */
#include <unistd.h>   /* POSIX operating system API (fork, pipe, dup2, execvp, chdir, getpid, getppid) */

#include <sys/types.h> /* system type definitions (pid_t) */
#include <sys/wait.h>  /* wait/waitpid declarations for child status monitoring */
#include <sys/stat.h>  /* file attributes and modes */

#include <fcntl.h>    /* file descriptor control operations (open, flags like O_RDONLY, O_WRONLY) */
#include <errno.h>    /* error codes for debugging failed system calls */
#include <limits.h>   /* environment/system limits definitions */

/*=============================================================================
    Project-Wide Constants
=============================================================================*/

/* Maximum character size of a single command line typed by the user */
#define SHELL_MAX_INPUT      128

/* Maximum number of arguments allowed in a single command */
#define SHELL_MAX_ARGS       64

/* Maximum number of commands stored in history for the 'hist' command */
#define SHELL_MAX_HISTORY    10

/* Maximum character length of the host machine's name buffer */
#define SHELL_HOSTNAME_SIZE  64

/* Maximum number of individual tokens (words/operators) per input line */
#define SHELL_MAX_TOKENS     64

/* Maximum character length of a single token text block */
#define SHELL_MAX_TOKEN_LENGTH 128

/*=============================================================================
    Global Variables
=============================================================================*/

/* 
 * Declares command_number variable. 
 * Allows all modules (main.c, history.c) to access/track the current command sequence.
 */
extern int command_number;

#endif

/******************************************************************************
 * File : builtin.c
 * Description : Built-in shell commands
 ******************************************************************************/

#include "../include/builtin.h"

/******************************************************************************
 * execute_builtin()
 *
 * Checks if the parsed command is a shell built-in. If it is, executes it
 * immediately and returns 1. Otherwise, returns 0.
 *
 * Supported Built-in Commands:
 *   - quit  : Exits the shell process cleanly.
 *   - cd    : Changes current working directory using chdir().
 *   - hist  : Displays the command history list.
 *   - curPid: Prints the Process ID (PID) of the current shell process.
 *   - pPid  : Prints the Process ID (PID) of the parent process of the shell.
 *
 * Parameters:
 *   - cmd: Pointer to the Command structure.
 *
 * Returns:
 *   - 1 if the command was a built-in, 0 otherwise.
 ******************************************************************************/
int execute_builtin(Command *cmd)
{
    /* If the command has no arguments, treat it as executed */
    if (cmd->argc == 0)
        return 1;

    /*-------------------------------------------------------------------------
        Command: quit
        Terminates the shell with success status.
    -------------------------------------------------------------------------*/
    if (strcmp(cmd->argv[0], "quit") == 0)
    {
        /* exit() terminates the calling process immediately */
        exit(EXIT_SUCCESS);
    }

    /*-------------------------------------------------------------------------
        Command: cd <path>
        Changes the current directory of the parent shell process.
    -------------------------------------------------------------------------*/
    if (strcmp(cmd->argv[0], "cd") == 0)
    {
        if (cmd->argc < 2)
        {
            /* If no path parameter is provided, print an operand error message */
            fprintf(stderr, "cd: missing operand\n");
        }
        else
        {
            /* 
             * chdir() is a system call that changes the current working directory.
             * Returns 0 on success, -1 on failure (e.g. directory does not exist).
             */
            if (chdir(cmd->argv[1]) != 0)
                perror("cd"); /* Print standard POSIX error details if failed */
        }

        return 1; /* Command handled */
    }

    /*-------------------------------------------------------------------------
        Command: hist
        Displays command history.
    -------------------------------------------------------------------------*/
    if (strcmp(cmd->argv[0], "hist") == 0)
    {
        /* Call history printer defined in history.c */
        show_history();
        return 1; /* Command handled */
    }

    /*-------------------------------------------------------------------------
        Command: curPid
        Prints the current process ID of the shell.
    -------------------------------------------------------------------------*/
    if (strcmp(cmd->argv[0], "curPid") == 0)
    {
        /* getpid() returns the process ID of the calling process */
        printf("Current Process ID : %d\n", getpid());
        return 1; /* Command handled */
    }

    /*-------------------------------------------------------------------------
        Command: pPid
        Prints the parent process ID of the shell.
    -------------------------------------------------------------------------*/
    if (strcmp(cmd->argv[0], "pPid") == 0)
    {
        /* getppid() returns the process ID of the parent of the calling process */
        printf("Parent Process ID : %d\n", getppid());
        return 1; /* Command handled */
    }

    /* Not a built-in command; needs to be fork/executed as an external program */
    return 0;
}

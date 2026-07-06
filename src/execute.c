/******************************************************************************
 * File        : execute.c
 * Description : Execute parsed commands (Built-in + External)
 ******************************************************************************/

#include "../include/execute.h"
#include "../include/builtin.h"

/******************************************************************************
 * execute_command()
 *
 * Executes a single parsed command (non-piped command).
 *
 * Steps:
 *   1. Check if it is a built-in command (like cd, quit, hist, curPid, pPid).
 *      Built-ins MUST run in the parent process to modify shell state.
 *   2. Fork a child process to run external/system commands (e.g. ls, pwd, grep).
 *   3. Inside the child:
 *      a. Set up input file redirection (<) if requested.
 *      b. Set up output file redirection (>) if requested.
 *      c. Replace process image using execvp().
 *   4. Inside the parent:
 *      Wait for the child process to complete execution to prevent zombies.
 *
 * Parameters:
 *   - cmd: Pointer to the Command structure to be executed.
 ******************************************************************************/
void execute_command(Command *cmd)
{
    pid_t pid; /* Variable to store the process ID resulting from fork */

    /* 
     * Safety check:
     * If command pointer is NULL or there are no arguments, do nothing.
     */
    if (cmd == NULL || cmd->argc == 0)
    {
        return;
    }

    /*------------------------------------------------------------
        Execute Built-in Commands
        (Must execute in parent process to affect shell environment)
    ------------------------------------------------------------*/
    if (execute_builtin(cmd))
    {
        /* If it was a built-in command, it is now executed. Return early. */
        return;
    }

    /*------------------------------------------------------------
        Create Child Process
        fork() duplicates the calling process, creating a new child.
    ------------------------------------------------------------*/
    pid = fork();

    if (pid < 0)
    {
        /* fork() returns -1 on error (e.g. system is out of memory/PIDs) */
        perror("fork");
        return;
    }

    /*============================================================
        Child Process Execution Context
        In the child process, fork() returns exactly 0.
    ============================================================*/
    if (pid == 0)
    {
        /*--------------------------------------------------------
            Input Redirection (<)
        --------------------------------------------------------*/
        if (cmd->input_file != NULL)
        {
            int fd;

            /* Open the source file in read-only mode */
            fd = open(cmd->input_file, O_RDONLY);

            if (fd < 0)
            {
                /* If file open fails (e.g. file does not exist), print error and exit child */
                perror(cmd->input_file);
                exit(EXIT_FAILURE);
            }

            /* 
             * dup2(fd, STDIN_FILENO) duplicates 'fd' onto file descriptor 0 (stdin).
             * This binds standard input of the command to read from the file.
             */
            if (dup2(fd, STDIN_FILENO) < 0)
            {
                perror("dup2");
                close(fd);
                exit(EXIT_FAILURE);
            }

            /* Close the original file descriptor to avoid descriptor leaks */
            close(fd);
        }

        /*--------------------------------------------------------
            Output Redirection (>)
        --------------------------------------------------------*/
        if (cmd->output_file != NULL)
        {
            int fd;

            /* 
             * Open/create the target file.
             * O_WRONLY: Write-only mode.
             * O_CREAT: Create the file if it does not exist.
             * O_TRUNC: Empty the file's contents before writing.
             * Permissions: 0644 (Read/Write for owner, Read for group/others).
             */
            fd = open(cmd->output_file,
                      O_WRONLY | O_CREAT | O_TRUNC,
                      0644);

            if (fd < 0)
            {
                /* If open fails (e.g. write-protected directory), print error and exit child */
                perror(cmd->output_file);
                exit(EXIT_FAILURE);
            }

            /* 
             * dup2(fd, STDOUT_FILENO) duplicates 'fd' onto file descriptor 1 (stdout).
             * This binds standard output of the command to write into the file.
             */
            if (dup2(fd, STDOUT_FILENO) < 0)
            {
                perror("dup2");
                close(fd);
                exit(EXIT_FAILURE);
            }

            /* Close the original file descriptor to avoid descriptor leaks */
            close(fd);
        }

        /*--------------------------------------------------------
            Execute External Command
            execvp() searches directories in the PATH environment variable
            for the command binary and replaces the current process image.
        --------------------------------------------------------*/
        execvp(cmd->argv[0], cmd->argv);

        /* 
         * execvp() only returns if an error occurred (e.g. command not found).
         * If it returns, print the error and exit the child with failure status.
         */
        perror(cmd->argv[0]);
        exit(EXIT_FAILURE);
    }

    /*============================================================
        Parent Process Execution Context
        In the parent process, fork() returns the Process ID (PID)
        of the newly created child process.
    ============================================================*/
    
    /* 
     * The parent shell blocks and waits for the child process to complete.
     * waitpid() blocks the calling process until the child specified by 'pid' terminates.
     */
    waitpid(pid, NULL, 0);
}

#ifndef EXECUTE_H
#define EXECUTE_H

#include "shell.h"
#include "command.h"

#include <fcntl.h>
#include <unistd.h>

/*=============================================================================
    execute_command() prototype:
    Executes a single parsed Command (non-piped). Checks built-ins first;
    otherwise forks a child process, manages redirs (< or >), and calls execvp.
=============================================================================*/
void execute_command(Command *cmd);

#endif

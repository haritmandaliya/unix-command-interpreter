#ifndef BUILTIN_H
#define BUILTIN_H

#include "shell.h"
#include "command.h"
#include "history.h"

/*=============================================================================
    execute_builtin() prototype:
    Tries to execute cmd as a built-in command (cd, quit, hist, curPid, pPid).
    Returns 1 if it is a built-in (and runs it), returns 0 otherwise.
=============================================================================*/
int execute_builtin(Command *cmd);

#endif

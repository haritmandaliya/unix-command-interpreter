#ifndef HISTORY_H
#define HISTORY_H

#include "shell.h"

/*=============================================================================
    add_history() prototype:
    Adds command text to the static history sliding window.
=============================================================================*/
void add_history(char *command);

/*=============================================================================
    show_history() prototype:
    Prints up to last 10 commands with overall command sequence numbers.
=============================================================================*/
void show_history(void);

#endif

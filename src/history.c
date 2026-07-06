/******************************************************************************
 * File : history.c
 * Description : Stores and prints last 10 commands in a sliding history window.
 ******************************************************************************/

#include "../include/history.h"

/*---------------------------------------------------------------------------
    History Storage:
    We define history variables as static so they are private to this file
    (encapsulated inside the history.c module) and cannot be modified by
    other code directly.
---------------------------------------------------------------------------*/

/* 2D array storing up to SHELL_MAX_HISTORY (10) command strings */
static char history[SHELL_MAX_HISTORY][SHELL_MAX_INPUT];

/* Keeps track of how many commands are currently stored in history (0 to 10) */
static int history_count = 0;

/******************************************************************************
 * add_history()
 *
 * Adds a new command to the history. If history is full, shifts existing
 * commands left (discarding the oldest command) to make room for the new one.
 *
 * Parameters:
 *   - command: The raw command string to add.
 * ****************************************************************************/
void add_history(char *command)
{
    int index;

    /* Ignore empty commands (no sense in storing empty lines in history) */
    if (command == NULL || strlen(command) == 0)
    {
        return;
    }

    /* 
     * Scenario A: History buffer has space available (< 10 items)
     */
    if (history_count < SHELL_MAX_HISTORY)
    {
        /* Copy command to the current end slot and increment count */
        strcpy(history[history_count], command);
        history_count++;
    }
    /* 
     * Scenario B: History buffer is full (already has 10 items)
     */
    else
    {
        /*
         * Shift everything one position to the left:
         * Index 0 gets value of Index 1 (oldest command is discarded)
         * Index 1 gets value of Index 2
         * ...
         * Index 8 gets value of Index 9
         */
        for (index = 0; index < SHELL_MAX_HISTORY - 1; index++)
        {
            strcpy(history[index], history[index + 1]);
        }

        /* Store the newest command in the last slot (Index 9) */
        strcpy(history[SHELL_MAX_HISTORY - 1], command);
    }
}

/******************************************************************************
 * show_history()
 *
 * Prints the stored history commands on the terminal.
 * Each command is printed along with its correct overall command sequence number.
 ******************************************************************************/
void show_history(void)
{
    int start_sequence;

    /*
     * Calculate the correct sequence number for the first item in the history.
     * Since the prompt displays the current command number as 'command_number' 
     * (and the current command is in the history buffer at index history_count - 1),
     * we align the sequence numbers so the last command printed is equal to the
     * current command number.
     *
     * Example:
     *   command_number = 25 (current command we are executing is #25)
     *   history_count = 10 (we have 10 commands in history)
     *   First command printed will have sequence: 25 - 10 + 1 = 16
     *   List printed will be: 16, 17, 18, 19, 20, 21, 22, 23, 24, 25.
     */
    start_sequence = command_number - history_count + 1;

    /* Loop and print all history items with their respective sequence number */
    for (int i = 0; i < history_count; i++)
    {
        printf("%d %s\n", start_sequence + i, history[i]);
    }
}

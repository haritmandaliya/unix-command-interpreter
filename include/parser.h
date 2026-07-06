#ifndef PARSER_H
#define PARSER_H

#include "shell.h"
#include "command.h"
#include "tokenizer.h"

/*=============================================================================
    build_command() prototype:
    Converts a sub-range of the token stream [start, end] into a Command struct.
    Returns the count of arguments (argc) populated.
=============================================================================*/
int build_command(Token tokens[], int start, int end, Command *cmd);

/*=============================================================================
    parse_tokens() prototype:
    Top-level parser function. Scans tokens for ';', validates grammar,
    and runs commands/pipelines sequentially.
=============================================================================*/
void parse_tokens(Token tokens[], int total);

#endif

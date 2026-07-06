#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "shell.h"

/*=============================================================================
    TokenType enumeration:
    Lists all category tags for lexed command line segments.
=============================================================================*/
typedef enum
{
    TOKEN_WORD,       /* A plain argument, parameter, command, or filename */
    TOKEN_PIPE,       /* The '|' character representing a pipeline */
    TOKEN_SEMICOLON,  /* The ';' character representing sequential command end */
    TOKEN_INPUT,      /* The '<' character representing standard input redirection */
    TOKEN_OUTPUT      /* The '>' character representing standard output redirection */
} TokenType;

/*=============================================================================
    Token structure:
    Represents an individual token in the command stream.
=============================================================================*/
typedef struct
{
    /* The literal text representing this token (e.g. "ls", "output.txt", "|") */
    char text[SHELL_MAX_TOKEN_LENGTH];

    /* The categoric type of the token */
    TokenType type;

} Token;

/*=============================================================================
    Tokenizer Function Prototype:
    Parses 'input' character string into 'tokens' array.
    Returns the total count of tokens parsed.
=============================================================================*/
int tokenize(char *input, Token tokens[]);

#endif

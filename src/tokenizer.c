#include "../include/tokenizer.h"
#include <ctype.h>

/******************************************************************************
 * tokenize()
 *
 * Lexical Analyzer: Converts a raw input string into a stream of Token structures.
 *
 * Parameters:
 *   - input: The raw command line string typed by the user.
 *   - tokens: An array of Token structures to be populated.
 *
 * Returns:
 *   - The total number of tokens identified in the string.
 ******************************************************************************/
int tokenize(char *input, Token tokens[])
{
    int i = 0;      /* Pointer/index to scan through the raw input string */
    int count = 0;  /* Index counter for the output tokens array */

    /* Scan characters one by one until we hit the null-terminator '\0' */
    while (input[i] != '\0')
    {
        /* 
         * Skip whitespace characters (spaces, tabs, carriage returns, etc.).
         * isspace() takes an int; casting to (unsigned char) avoids issues
         * with negative signed char values.
         */
        if (isspace((unsigned char)input[i]))
        {
            i++; /* Advance input index */
            continue; /* Skip to next iteration */
        }

        /* 
         * Detect a Pipe operator '|'
         */
        if (input[i] == '|')
        {
            strcpy(tokens[count].text, "|");  /* Copy the token text */
            tokens[count].type = TOKEN_PIPE;  /* Tag it as a PIPE token */

            count++; /* Increment token counter */
            i++;     /* Move past the pipe character in input */
            continue; /* Process next token */
        }

        /* 
         * Detect a Semicolon operator ';'
         */
        if (input[i] == ';')
        {
            strcpy(tokens[count].text, ";");       /* Copy the token text */
            tokens[count].type = TOKEN_SEMICOLON;  /* Tag it as a SEMICOLON token */

            count++; /* Increment token counter */
            i++;     /* Move past the semicolon character in input */
            continue; /* Process next token */
        }

        /* 
         * Detect an Input Redirection operator '<'
         */
        if (input[i] == '<')
        {
            strcpy(tokens[count].text, "<");   /* Copy the token text */
            tokens[count].type = TOKEN_INPUT;  /* Tag it as an INPUT redirection token */

            count++; /* Increment token counter */
            i++;     /* Move past the '<' character in input */
            continue; /* Process next token */
        }

        /* 
         * Detect an Output Redirection operator '>'
         */
        if (input[i] == '>')
        {
            strcpy(tokens[count].text, ">");    /* Copy the token text */
            tokens[count].type = TOKEN_OUTPUT;  /* Tag it as an OUTPUT redirection token */

            count++; /* Increment token counter */
            i++;     /* Move past the '>' character in input */
            continue; /* Process next token */
        }

        /* 
         * If the character was not a space or any operator, it must be a WORD.
         * A word is an argument or command name (e.g. "ls", "-la", "file.txt").
         */
        int j = 0; /* Index to construct the individual word's text */

        /* 
         * Read characters of the word until we hit a whitespace character 
         * or any shell operator (| ; < >) which marks the end of the word.
         */
        while (input[i] &&
               !isspace((unsigned char)input[i]) &&
               input[i] != '|' &&
               input[i] != ';' &&
               input[i] != '<' &&
               input[i] != '>')
        {
            /* Copy character into token's text buffer and advance indices */
            tokens[count].text[j++] = input[i++];
        }

        /* Null-terminate the constructed word to make it a valid C string */
        tokens[count].text[j] = '\0';
        
        /* Tag the token type as a normal WORD */
        tokens[count].type = TOKEN_WORD;

        count++; /* Move to the next slot in the tokens array */
    }

    /* Return the total count of tokens found */
    return count;
}

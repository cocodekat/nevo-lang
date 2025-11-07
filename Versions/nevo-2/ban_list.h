#ifndef BAN_LIST_H
#define BAN_LIST_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Whitelisted functions / keywords
const char* white_list[] = {
    "rand",
    "print(",
    "get(",
    "kaboom(",
    "stop(",
    "exit(",
    "arradd(",
    "carr(",
    "parr(",
    "farr(",
    "gets(",
    "h1(",
    "sha256(",
    "bin(",
    "pbin("
    "switch(",
    "case(",
    "if",
    "else",
    "else if",
    "while",
    "for",
    "do",
    "++",
    "--",
    "{",
    "return",
    "int ",
    "char ",
    "float ",
    "double ",
    "long ",
    "short ",
    "unsigned ",
    "void ",
    "srand(",
    "= ("
};

const int white_count = sizeof(white_list) / sizeof(white_list[0]);

// Types used to identify user-defined functions
const char* types[] = { "int", "char", "float", "double", "long", "short", "unsigned", "void" };
const int types_count = sizeof(types) / sizeof(types[0]);

static void check_ban_list(const char* line) {
    // 1. Allow empty lines or preprocessor directives
    if (!line || line[0] == '\0' || line[0] == '#') return;

    const char* p = line;

    // skip leading spaces/tabs
    while (*p && isspace((unsigned char)*p)) p++;

    // allow opening and closing braces
    if (*p == '{' || *p == '}') return;

    // 2. Check whitelist
    for (int i = 0; i < white_count; i++) {
        if (strstr(line, white_list[i])) {
            return; // allowed
        }
    }

    // 3. Check for user-defined function declaration
    // Must start with a type keyword, and have a '(' after the name
    for (int i = 0; i < types_count; i++) {
        size_t type_len = strlen(types[i]);
        if (strncmp(p, types[i], type_len) == 0 && isspace((unsigned char)p[type_len])) {
            // Look for '(' after the function name
            const char* name_start = p + type_len;
            while (*name_start && isspace((unsigned char)*name_start)) name_start++;

            // Skip valid function name characters
            const char* name_end = name_start;
            while (*name_end && (isalnum((unsigned char)*name_end) || *name_end == '_')) name_end++;

            if (*name_end == '(') {
                return; // valid user-defined function
            }
        }
    }

    // 4. If nothing matched → unrecognized function/keyword
    fprintf(stderr, "Error: Unrecognized function or keyword found in line: %s\n", line);
    exit(1);
}

#endif

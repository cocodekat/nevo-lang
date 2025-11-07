#ifndef BAN_LIST_H
#define BAN_LIST_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// --- Whitelisted functions / keywords ---
const char* white_list[] = {
    // --- Built-in functions ---
    "arradd(", "h1_bin(", "sha256_bin(", "carr(", "exit(", "farr(", "get(",
    "gets(", "h1(", "kaboom(", "pbin(", "parr(", "print(", "rand", "srand(",
    "sha256(", "stop(", "switch(",

    // --- File operations ---
    "fread(", "fwritef(", "freadf(", "fopen(", "fclose(",

    // --- Image operations ---
    "image(", "compile(", "depile(",

    // --- Control flow keywords ---
    "case(", "do", "else", "else if", "for", "if", "while", "?", ":",

    // --- Operators and symbols ---
    "++", "--", "= (", "{",

    // --- Other language keywords ---
    "return", "int ", "char ", "float ", "double ", "long ", "short ",
    "unsigned ", "void ",

    "int8_t", "int16_t", "int32_t", "int64_t", "int128_t",
    "uint8_t", "uint16_t", "uint32_t", "uint64_t", "uint128_t",

    "#define",
    "typedef"
};

const int white_count = sizeof(white_list) / sizeof(white_list[0]);

// --- Known base types for function declarations ---
const char* types[] = { "int", "char", "float", "double", "long", "short", "unsigned", "void" };
const int types_count = sizeof(types) / sizeof(types[0]);

// --- Dynamic typedef registry ---
#define MAX_TYPEDEFS 100
static char* typedef_names[MAX_TYPEDEFS];
static int typedef_count = 0;

// --- Ban list checker ---
static void check_ban_list(const char* line) {
    if (!line || line[0] == '\0' || line[0] == '#')
        return;

    const char* p = line;
    while (*p && isspace((unsigned char)*p)) p++;

    // Allow braces
    if (*p == '{' || *p == '}') return;

    // --- 1. Detect typedef lines ---
    if (strncmp(p, "typedef", 7) == 0 && isspace((unsigned char)p[7])) {
        const char* alias = strrchr(p, ' ');
        if (alias && typedef_count < MAX_TYPEDEFS) {
            alias++; // move past space
            char* clean = strdup(alias);
            clean[strcspn(clean, ";\r\n")] = '\0'; // trim semicolon/newline
            typedef_names[typedef_count++] = clean;
        }
        return; // typedef lines are always valid
    }

    // --- 2. Whitelist check ---
    for (int i = 0; i < white_count; i++) {
        if (strstr(line, white_list[i])) {
            return; // allowed keyword/function
        }
    }

    // --- 3. Check if line starts with a typedef-defined alias ---
    for (int i = 0; i < typedef_count; i++) {
        size_t name_len = strlen(typedef_names[i]);
        if (strncmp(p, typedef_names[i], name_len) == 0 &&
            isspace((unsigned char)p[name_len])) {
            return; // valid typedef alias
        }
    }

    // --- 4. Check for user-defined function declaration ---
    for (int i = 0; i < types_count; i++) {
        size_t type_len = strlen(types[i]);
        if (strncmp(p, types[i], type_len) == 0 && isspace((unsigned char)p[type_len])) {
            const char* name_start = p + type_len;
            while (*name_start && isspace((unsigned char)*name_start)) name_start++;

            const char* name_end = name_start;
            while (*name_end && (isalnum((unsigned char)*name_end) || *name_end == '_')) name_end++;

            if (*name_end == '(') return; // function declaration
        }
    }

    // --- 5. Nothing matched → error ---
    fprintf(stderr, "Error: Unrecognized function or keyword found in line: %s\n", line);
    exit(1);
}

#endif

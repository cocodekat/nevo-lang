#ifndef CHECK_IMPORTS_H
#define CHECK_IMPORTS_H

#include <stdbool.h>
#include <string.h>

typedef struct {
    bool stdio;
    bool stdlib;
    bool string;
    bool math;
    bool ctype;
    bool time;
    bool assert;
    bool stdbool;
    bool arradd;
    bool h1;
    bool sha256;
    bool npxm;
} headers_needed_t;

static void calculate_needed_headers(const char *code_lines[], int num_lines, headers_needed_t *out) {
    out->stdio = false;
    out->stdlib = false;
    out->string = false;
    out->math = false;
    out->ctype = false;
    out->time = false;
    out->assert = false;
    out->stdbool = false;
    out->arradd = false;
    out->h1 = false;
    out->sha256 = false;
    out->npxm = false;

    const char *stdio_funcs[] = {"printf","scanf","fprintf","fscanf","sprintf","sscanf","snprintf","putchar","getchar","puts","gets","fgets","fputs"};
    const char *stdlib_funcs[] = {"malloc","calloc","realloc","free","atoi","atol","atoll","strtol","strtoll","strtoul","strtoull","strtod","strtof","strtold","exit","abort","system","rand","srand","qsort","bsearch"};
    const char *string_funcs[] = {"strlen","strcpy","strncpy","strcat","strncat","strcmp","strncmp","strchr","strrchr","strstr","strtok","strspn","strcspn","strpbrk","memset","memcpy","memmove","memcmp","memchr"};
    const char *math_funcs[] = {"sin","cos","tan","asin","acos","atan","atan2","sinh","cosh","tanh","asinh","acosh","atanh","exp","exp2","expm1","log","log10","log2","log1p","pow","sqrt","cbrt","hypot","ceil","floor","trunc","round","lround","llround","rint","lrint","llrint","fmod","remainder","fabs"};
    const char *ctype_funcs[] = {"isalnum","isalpha","isdigit","islower","isupper","isspace","tolower","toupper"};
    const char *time_funcs[] = {"time","clock","difftime","mktime","strftime","localtime","gmtime"};
    const char *assert_funcs[] = {"assert"};
    const char *stdbool_funcs[] = {"bool","true","false"};
    const char* arradd_funcs[] = {"arradd","parr","farr","_int","_str","float","double","char","bool","carr"};
    const char* h1_funcs[] = {"h1"};
    const char* sha256_funcs[] = {"sha256"};
    const char* npxm_funcs[] = {"image"};

    for (int i = 0; i < num_lines; i++) {
        const char *line = code_lines[i];
        for (int j = 0; j < sizeof(stdio_funcs)/sizeof(stdio_funcs[0]); j++)
            if (strstr(line, stdio_funcs[j])) out->stdio = true;
        for (int j = 0; j < sizeof(stdlib_funcs)/sizeof(stdlib_funcs[0]); j++)
            if (strstr(line, stdlib_funcs[j])) out->stdlib = true;
        for (int j = 0; j < sizeof(string_funcs)/sizeof(string_funcs[0]); j++)
            if (strstr(line, string_funcs[j])) out->string = true;
        for (int j = 0; j < sizeof(math_funcs)/sizeof(math_funcs[0]); j++) {
            char func_call[64];
            snprintf(func_call, sizeof(func_call), "%s(", math_funcs[j]);
            if (strstr(line, func_call)) out->math = true;
        }
        for (int j = 0; j < sizeof(ctype_funcs)/sizeof(ctype_funcs[0]); j++)
            if (strstr(line, ctype_funcs[j])) out->ctype = true;

        for (int j = 0; j < sizeof(time_funcs)/sizeof(time_funcs[0]); j++)
            if (strstr(line, time_funcs[j])) out->time = true;

        for (int j = 0; j < sizeof(assert_funcs)/sizeof(assert_funcs[0]); j++)
            if (strstr(line, assert_funcs[j])) out->assert = true;

        for (int j = 0; j < sizeof(stdbool_funcs)/sizeof(stdbool_funcs[0]); j++)
            if (strstr(line, stdbool_funcs[j])) out->stdbool = true;

        // #----Builtin funcs

        for (int j = 0; j < sizeof(arradd_funcs)/sizeof(arradd_funcs[0]); j++)
            if (strstr(line, arradd_funcs[j])) out->arradd = true;

        for (int j = 0; j < sizeof(h1_funcs)/sizeof(h1_funcs[0]); j++)
            if (strstr(line, h1_funcs[j])) out->h1 = true;

        for (int j = 0; j < sizeof(sha256_funcs)/sizeof(sha256_funcs[0]); j++)
            if (strstr(line, sha256_funcs[j])) out->sha256 = true;
        
        for (int j = 0; j < sizeof(npxm_funcs)/sizeof(npxm_funcs[0]); j++)
            if (strstr(line, npxm_funcs[j])) out->npxm = true;

    }
}

#endif

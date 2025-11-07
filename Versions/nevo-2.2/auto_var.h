#ifndef AUTO_VAR_H
#define AUTO_VAR_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>

// ---------------- Variable tracking ----------------

#define MAX_VARS 256

static char declared_vars[MAX_VARS][128];
static int declared_count = 0;

static int is_declared(const char *name) {
    for (int i = 0; i < declared_count; i++) {
        if (strcmp(declared_vars[i], name) == 0) return 1;
    }
    return 0;
}

static void add_var(const char *name) {
    if (!is_declared(name) && declared_count < MAX_VARS) {
        strncpy(declared_vars[declared_count++], name, sizeof(declared_vars[0]) - 1);
        declared_vars[declared_count-1][sizeof(declared_vars[0]) - 1] = '\0';
    }
}

// ---------------- String utils ----------------

static void trim(char *s) {
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start) + 1);

    char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) *end-- = '\0';
}

static int next_nonspace_idx(const char *s, int idx) {
    int i = idx;
    while (s[i]) {
        if (!isspace((unsigned char)s[i])) return i;
        i++;
    }
    return -1;
}

static int prev_nonspace_idx(const char *s, int idx) {
    int i = idx;
    while (i >= 0) {
        if (!isspace((unsigned char)s[i])) return i;
        i--;
    }
    return -1;
}

// ---------------- Type inference ----------------

static const char *infer_type(const char *rhs) {
    char buf[1024];
    strncpy(buf, rhs, sizeof(buf));
    buf[sizeof(buf)-1] = '\0';
    trim(buf);

    int len = strlen(buf);

    if (len > 1 && (buf[len-1] == 'f' || buf[len-1] == 'F')) return "float";
    for (int i = 0; buf[i]; i++) if (buf[i] == '.') return "float";

    int all_digits = 1, has_digit = 0;
    for (int i = 0; buf[i]; i++) {
        if (isdigit((unsigned char)buf[i])) has_digit = 1;
        else if (!isspace((unsigned char)buf[i]) &&
                 buf[i] != '+' && buf[i] != '-' &&
                 buf[i] != '*' && buf[i] != '/' &&
                 buf[i] != '(' && buf[i] != ')' &&
                 buf[i] != '%') {
            all_digits = 0; break;
        }
    }
    if (has_digit && all_digits) return "int";

    if (strstr(buf, "rand(")) return "int";
    if (strstr(buf, "time(")) return "time_t";
    if (strstr(buf, "malloc(")) return "void*";
    if (strstr(buf, "carr(")) return "Array*";
    if (strstr(buf, "h1(")) return "int";
    if (strstr(buf, "sha256(")) return "char*";
    if (strstr(buf, "cap")) return "bool";
    if (strstr(buf, "nocap")) return "bool";

    return "float";
}

// ---------------- Main core: everything in check_variable ----------------

static void check_variable(char *line) {
    trim(line);
    if (strlen(line) == 0) return;

    // Skip lines that already start with a type
    const char *types[] = {
        // Basic types
        "void",
        "char",
        "short",
        "int",
        "long",
        "float",
        "double",
        "unsigned",
        "time_t",
        "size_t",
        "Array",
        "FILE",

        // Fixed-width integer types
        "int8_t", "int16_t", "int32_t", "int64_t", "int128_t",
        "uint8_t", "uint16_t", "uint32_t", "uint64_t", "uint128_t",

        // Pointer versions of basic types
        "void*",
        "char*",
        "short*",
        "int*",
        "long*",
        "float*",
        "double*",
        "unsigned*",
        "time_t*",
        "size_t*",
        "Array*",
        "FILE*",

        // Pointer versions of fixed-width integer types
        "int8_t*",
        "int16_t*",
        "int32_t*",
        "int64_t*",
        "int128_t*",
        "uint8_t*",
        "uint16_t*",
        "uint32_t*",
        "uint64_t*",
        "uint128_t*"
    };

    for (int i = 0; i < (int)(sizeof(types)/sizeof(types[0])); i++) {
        size_t len = strlen(types[i]);
        if (strncmp(line, types[i], len) == 0 &&
            (line[len] == '\0' || isspace((unsigned char)line[len]) || line[len] == '*')) {
            // extract the var name from declaration
            char tmp[128];
            const char *p = line + len;
            while (*p && isspace((unsigned char)*p)) p++;
            int j = 0;
            while (*p && (isalnum((unsigned char)*p) || *p == '_' || *p == '*')) {
                if (j < (int)sizeof(tmp)-1) tmp[j++] = *p;
                p++;
            }
            tmp[j] = '\0';
            trim(tmp);
            add_var(tmp);
            return;
        }
    }

    // Keyword skip
    const char *keywords[] = {"if","else","else if","for","while","switch","return"};
    for (int i = 0; i < (int)(sizeof(keywords)/sizeof(keywords[0])); i++) {
        size_t len = strlen(keywords[i]);
        if (strncmp(line, keywords[i], len) == 0 &&
            (line[len] == '\0' || isspace((unsigned char)line[len]) || line[len] == '(')) {
            return;
        }
    }

    // Look for assignment '=' (but not ==, !=, <=, >=)
    int eq_pos = -1;
    int be_pos = -1;

    for (int i = 0; line[i]; i++) {
        // '=' operator
        if (line[i] == '=') {
            int next = next_nonspace_idx(line, i+1);
            int prev = prev_nonspace_idx(line, i-1);
            if (next != -1 && line[next] == '=') continue;
            if (prev != -1 && (line[prev] == '!' || line[prev] == '<' || line[prev] == '>')) continue;
            eq_pos = i;
            break;
        }

        // 'be' operator
        if (line[i] == 'b' && line[i+1] == 'e') {
            be_pos = i;
            break;
        }
    }

    if (eq_pos != -1) {
        // process '=' assignment
    } else if (be_pos != -1) {
        eq_pos = be_pos;
        // replace 'be' with '=' for RHS extraction
        line[eq_pos] = '=';
        memmove(line + eq_pos + 1, line + eq_pos + 2, strlen(line + eq_pos + 2) + 1);
    } else {
        return; // no assignment found
    }


    // Split into var and rhs
    char var[128], rhs[512];
    strncpy(var, line, eq_pos);
    var[eq_pos] = '\0';
    strcpy(rhs, line + eq_pos + 1);
    trim(var);
    trim(rhs);

    // Ensure it's a valid identifier
    if (strlen(var) == 0) return;
    int valid_ident = 1;
    int start = 0;
    if (var[start] == '*') start++;
    for (int i = start; var[i]; i++) {
        if (!(isalnum((unsigned char)var[i]) || var[i] == '_')) { valid_ident = 0; break; }
    }
    if (!valid_ident) return;

    // If already declared (global or earlier), just leave assignment as-is
    if (is_declared(var)) return;

    // Otherwise, infer type and turn it into a declaration
    const char *type = infer_type(rhs);
    char new_line[1024];
    snprintf(new_line, sizeof(new_line), "%s %s = %s;", type, var, rhs);
    strcpy(line, new_line);

    add_var(var);
}

#endif // AUTO_VAR_H

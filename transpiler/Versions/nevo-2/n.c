#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "check_imports.h"
#include "auto_var.h"
#include "ban_list.h"
#include "replacements.h"

const int replacements_count = sizeof(replacements) / sizeof(replacement_t);

void apply_replacements(char *line) {
    char buffer[1024];
    for (int r = 0; r < replacements_count; r++) {
        char *src = line;
        char *dst = buffer;
        buffer[0] = '\0';

        while (*src) {
            char *pos = strstr(src, replacements[r].from);
            if (pos) {
                int len = pos - src;
                memcpy(dst, src, len);
                dst += len;

                strcpy(dst, replacements[r].to);
                dst += strlen(replacements[r].to);

                src = pos + strlen(replacements[r].from);
            } else {
                strcpy(dst, src);
                break;
            }
        }

        strcpy(line, buffer); // copy back into original line
    }
}

void rtrim(char *s) {
    int len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[--len] = '\0';
    }
}

void remove_comments(char *line) {
    // Remove inline // comments
    char *comment = strstr(line, "//");
    if (comment) {
        *comment = '\0';
    }

    // Remove block /* */ comments (simple single-line version)
    char *start = strstr(line, "/*");
    while (start) {
        char *end = strstr(start + 2, "*/");
        if (end) {
            memmove(start, end + 2, strlen(end + 2) + 1); // remove comment
        } else {
            *start = '\0'; // no closing */, remove till end of line
            break;
        }
        start = strstr(line, "/*"); // check again
    }

    rtrim(line); // trim trailing spaces
}

int needs_semicolon(const char *line) {
    char tmp[1024];
    strncpy(tmp, line, sizeof(tmp));
    tmp[sizeof(tmp)-1] = 0;

    rtrim(tmp);

    if (strlen(tmp) == 0) return 0;
    if (tmp[0] == '#') return 0;
    if (tmp[0] == '/' && tmp[1] == '/') return 0;

    // Cut off inline comment
    char *comment = strstr(tmp, "//");
    if (comment) *comment = '\0';

    rtrim(tmp);  // trim again after cutting comment

    if (strlen(tmp) == 0) return 0;

    char last = tmp[strlen(tmp)-1];
    if (last == '{' || last == '}' || last == ';') return 0;

    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input file> [input file2 ...]\n", argv[0]);
        return 1;
    }

    // Process each file in argv[1..argc-1]
    for (int file_idx = 1; file_idx < argc; file_idx++) {
        char *input_file = argv[file_idx];
        FILE *in = fopen(input_file, "r");
        if (!in) {
            perror("fopen input");
            continue; // skip to next file instead of aborting everything
        }

        // Determine output filename
        char output_file[256];
        strncpy(output_file, input_file, sizeof(output_file));
        output_file[sizeof(output_file)-1] = '\0';
        char *dot = strrchr(output_file, '.');
        if (dot) strcpy(dot, ".c");
        else strcat(output_file, ".c");

        // Read all lines into memory
        char *lines[1000];
        int num_lines = 0;
        char line[1024];

        while (fgets(line, sizeof(line), in)) {
            rtrim(line);
            remove_comments(line);

            char tmp[1024];
            strncpy(tmp, line, sizeof(tmp));
            tmp[sizeof(tmp)-1] = 0;

            check_variable(tmp);
            check_ban_list(tmp);
            apply_replacements(tmp);

            lines[num_lines++] = strdup(tmp);
        }
        fclose(in);

        // Open output file
        FILE *out = fopen(output_file, "w");
        if (!out) {
            perror("fopen output");
            for (int i = 0; i < num_lines; i++) free(lines[i]);
            continue;   
        }

        // Calculate required headers
        headers_needed_t needed;
        calculate_needed_headers((const char **)lines, num_lines, &needed);

        // Write headers
        if (needed.stdio) fprintf(out, "#include <stdio.h>\n");
        if (needed.stdlib) fprintf(out, "#include <stdlib.h>\n");
        if (needed.string) fprintf(out, "#include <string.h>\n");
        if (needed.math) fprintf(out, "#include <math.h>\n");
        if (needed.ctype) fprintf(out, "#include <ctype.h>\n");
        if (needed.time) fprintf(out, "#include <time.h>\n");
        if (needed.assert) fprintf(out, "#include <assert.h>\n");
        if (needed.stdbool) fprintf(out, "#include <stdbool.h>\n");
        if (needed.arradd) {
            #if defined(_WIN32) || defined(_WIN64)
                fprintf(out, "#include \"C:\\nevo\\libraries\\arradd.h\"\n");
            #else
                fprintf(out, "#include \"%s/nevo/libraries/arradd.h\"\n", getenv("HOME"));
            #endif
        }
        if (needed.h1) {
            #if defined(_WIN32) || defined(_WIN64)
                fprintf(out, "#include \"C:\\nevo\\libraries\\h1.h\"\n");
            #else
                fprintf(out, "#include \"%s/nevo/libraries/h1.h\"\n", getenv("HOME"));
            #endif
        }
        if (needed.sha256) {
            #if defined(_WIN32) || defined(_WIN64)
                fprintf(out, "#include \"C:\\nevo\\libraries\\sha256.h\"\n");

            #else
                fprintf(out, "#include \"%s/nevo/libraries/sha256.h\"\n", getenv("HOME"));
            #endif
        }


        for (int i = 0; i < num_lines; i++) {
            char *l = lines[i];
            fprintf(out, "%s\n", l);
            free(l);
        }

        fclose(out);
        printf("Processed %s -> %s\n", input_file, output_file);

        // Use clang instead of GCC
        #if defined(_WIN32) || defined(_WIN64)
            const char *compiler = "C:\\nevo\\mingw64\\mingw64\\bin\\gcc.exe";
        #else
            const char *compiler = "clang";
        #endif
        char cmd[512];
        
        #if defined(_WIN32) || defined(_WIN64)
            snprintf(cmd, sizeof(cmd), "%s %s -o %.*s", compiler, output_file, (int)(dot - output_file), output_file);
        #else 
            snprintf(cmd, sizeof(cmd), "%s %s -o %.*s > /dev/null 2>&1",compiler, output_file, (int)(dot - output_file), output_file);
        #endif
        
        int ret = system(cmd);
        if (ret == 0) {
            printf("Compilation succeeded: %.*s\n", (int)(dot - output_file), output_file);
        } else {
            printf("Compilation failed with code %d\n", ret);
        }
        remove(output_file);

    }

    return 0;
}

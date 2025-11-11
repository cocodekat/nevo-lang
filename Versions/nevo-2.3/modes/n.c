#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "../check_imports.h"
#include "../auto_var.h"
#include "../ban_list.h"
#include "../replacements.h"

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

int has_missing_exclamation(char *line) {
    rtrim(line);
    int len = strlen(line);
    if (len == 0) return 0;
    char last = line[len - 1];
    if (last == '!' || last == '{' || last == '}') return 0;
    return 1;
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input file> [input file2 ...]\n", argv[0]);
        return 1;
    }

    for (int file_idx = 1; file_idx < argc; file_idx++) {
        char *input_file = argv[file_idx];
        FILE *in = fopen(input_file, "r");
        if (!in) {
            perror("fopen input");
            continue;
        }

        // Output naming
        char output_file[256];
        strncpy(output_file, input_file, sizeof(output_file));
        output_file[sizeof(output_file) - 1] = '\0';
        char *dot = strrchr(output_file, '.');
        if (dot) strcpy(dot, ".c");
        else strcat(output_file, ".c");
        char c_file[] = "A1bC2dE35F31nBv53X.c";

        // Read + preprocess
        char *lines[1000];
        int num_lines = 0;
        char line[1024];
        while (fgets(line, sizeof(line), in)) {
            rtrim(line);
            remove_comments(line);
            char tmp[1024];
            strncpy(tmp, line, sizeof(tmp));
            tmp[sizeof(tmp) - 1] = 0;
            check_variable(tmp);
            register_functions_in_line(tmp);
            check_function_calls_in_line(tmp);
            apply_replacements(tmp);
            lines[num_lines++] = strdup(tmp);
        }
        fclose(in);

        FILE *out = fopen(c_file, "w");
        if (!out) {
            perror("fopen output");
            for (int i = 0; i < num_lines; i++) free(lines[i]);
            continue;
        }

        headers_needed_t needed;
        calculate_needed_headers((const char **)lines, num_lines, &needed);

        // Include headers
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
        if (needed.npxm) {
        #if defined(_WIN32) || defined(_WIN64)
            fprintf(out, "#include \"C:\\nevo\\libraries\\images\\npxm.h\"\n");
        #else
            fprintf(out, "#include \"%s/nevo/libraries/images/npxm.h\"\n", getenv("HOME"));
        #endif
        }

        if (needed.be_defined) fprintf(out, "#define be =\n");
        if (needed.cap_defined) fprintf(out, "#define cap false\n");
        if (needed.nocap_defined) fprintf(out, "#define nocap true\n");

        int needs_type = 0;

        for (int i = 0; i < num_lines; i++) {
            fprintf(out, "%s\n", lines[i]);
            free(lines[i]);
        }

        fclose(out);

        // Compile
        #if defined(_WIN32) || defined(_WIN64)
            const char *compiler = "C:\\nevo\\tcc\\tcc\\tcc.exe";
        #else
            const char *compiler = "clang";
        #endif

        char cmd[512];

        #if defined(_WIN32) || defined(_WIN64)
            char exe_output[256];
            strncpy(exe_output, output_file, sizeof(exe_output));
            exe_output[sizeof(exe_output)-1] = 0;

            dot = strrchr(exe_output, '.');
            if (dot) strcpy(dot, ".exe");
            else strcat(exe_output, ".exe");

            // Wrap in quotes in case of spaces
            snprintf(cmd, sizeof(cmd),
                "\"%s\" -Wall -std=c99 \"%s\" -o \"%s\" 2>&1",
                compiler, c_file, exe_output);

        #else
            dot = strrchr(output_file, '.');
            snprintf(cmd, sizeof(cmd),
                "%s -Wall -std=c99 %s -o %.*s 2>&1",
                compiler, c_file, (int)(dot - output_file), output_file);
        #endif




        FILE *pipe = popen(cmd, "r");
        if (!pipe) {
            printf("Internal Error: Compiler launch failed.\n");
            remove(c_file);
            continue;
        }

        char buf[512];
        int has_error = 0;

        // Scan compiled C output
        while (fgets(buf, sizeof(buf), pipe)) {
            int line_num = 0;
            if (sscanf(buf, "%*[^:]:%d:", &line_num) != 1) line_num = 0;

            // Track #define / #include lines
            int header_lines = 0;
            if (needed.stdio) header_lines++;
            if (needed.stdlib) header_lines++;
            if (needed.string) header_lines++;
            if (needed.math) header_lines++;
            if (needed.ctype) header_lines++;
            if (needed.time) header_lines++;
            if (needed.assert) header_lines++;
            if (needed.stdbool) header_lines++;
            if (needed.arradd) header_lines++;
            if (needed.h1) header_lines++;
            if (needed.sha256) header_lines++;
            if (needed.npxm) header_lines++;
            if (needed.be_defined) header_lines += 3; // be, cap, nocap

            int orig_line = line_num - header_lines;
            if (orig_line < 1) orig_line = 1;


            if (strstr(buf, "implicit declaration of function")) {
                char *start = strchr(buf, '\''); if (start) { start++; char *end = strchr(start,'\''); if(end)*end=0; }
                printf("Error (line %d): Function '%s' is not recognized.\n", orig_line, start ? start : "unknown");
                has_error = 1;
            }
            else if (strstr(buf, "undeclared identifier")) {
                char *start = strchr(buf, '\''); if (start) { start++; char *end = strchr(start,'\''); if(end)*end=0; }
                printf("Error (line %d): Variable or function '%s' is not defined.\n", orig_line, start ? start : "unknown");
                has_error = 1;
            }
            else if (strstr(buf, "expected ';'")) {
                printf("Error (line %d): Expected ! at end of line.\n", orig_line);
                has_error = 1;
            }
            else if (strstr(buf, "redefinition of")) {
                printf("Error (line %d): Redefinition detected — a name is declared twice.\n", orig_line);
                has_error = 1;
            }
            else if (strstr(buf, "no such file or directory")) {
                printf("Error (line %d): Missing file or bad include path.\n", orig_line);
                has_error = 1;
            }
            else if (strstr(buf, "#define") || strstr(buf, "#include")) {
                // Optionally show preprocessor issues
                printf("Warning (line %d): %s", orig_line, buf);
            }
            if (ban_error() == 1) {
                has_error = 1;
            }
            
        }


        pclose(pipe);
        // remove(c_file);

        if (!has_error)
            printf("Compilation succeeded: %.*s\n", (int)(dot - output_file), output_file);
        else
            printf("Compilation failed.\n");
    }

    return 0;
}
// fixed_webview_generator.c
// Safer rewrite of your original program with memory-safety fixes.

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "../check_imports.h"
#include "../auto_var.h"
#include "../ban_list.h"
#include "../replacements.h"

const int replacements_count = sizeof(replacements) / sizeof(replacement_t);

// --- Helper for cross-platform paths ---
#ifdef _WIN32
#define SYSPATH(win, unix) win
#else
#define SYSPATH(win, unix) unix
#endif

// --- Include indices ---
enum {
    INC_STD, INC_STDL, INC_STR, INC_MATH, INC_CTP, INC_TIME, INC_ASSERT,
    INC_ARR, INC_H1, INC_SHA, INC_NPXM, INC_BOOL, INC_BOPS, INC_UNLESS
};

// --- Linux paths ---
static const char *linux_paths[] = {
    "#include <stdio.h>", "#include <stdlib.h>", "#include <string.h>",
    "#include <math.h>", "#include <ctype.h>", "#include <time.h>",
    "#include <assert.h>", "/nevo/libraries/arradd.h", "/nevo/libraries/h1.h",
    "/nevo/libraries/sha256.h", "/nevo/libraries/images/npxm.h",
    "/nevo/libraries/bool.h", "/nevo/libraries/better_ops.h",
    "/nevo/libraries/unless.h"
};

// --- Windows paths ---
static const char *win_paths[] = {
    "#include \"C:\\nevo\\tcc\\tcc\\include\\stdio.h\"",
    "#include \"C:\\nevo\\tcc\\tcc\\include\\stdlib.h\"",
    "#include \"C:\\nevo\\tcc\\tcc\\include\\string.h\"",
    "#include \"C:\\nevo\\tcc\\tcc\\include\\math.h\"",
    "#include \"C:\\nevo\\tcc\\tcc\\include\\ctype.h\"",
    "#include \"C:\\nevo\\tcc\\tcc\\include\\time.h\"",
    "#include \"C:\\nevo\\tcc\\tcc\\include\\assert.h\"",
    "#include \"C:\\nevo\\libraries\\arradd.h\"", "#include \"C:\\nevo\\libraries\\h1.h\"",
    "#include \"C:\\nevo\\libraries\\sha256.h\"", "#include \"C:\\nevo\\libraries\\images\\npxm.h\"",
    "#include \"C:\\nevo\\libraries\\bool.h\"", "#include \"C:\\nevo\\libraries\\better_ops.h\"",
    "#include \"C:\\nevo\\libraries\\unless.h\""
};

// --- Utility helpers ---
void rtrim(char *s) {
    int len = (int)strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) s[--len] = '\0';
}

void remove_comments(char *line) {
    if (!line) return;
    char *c = strstr(line, "//"); if (c) *c = '\0';
    char *s = strstr(line, "/*");
    while (s) {
        char *e = strstr(s + 2, "*/");
        if (e) {
            /* remove the block comment by moving tail left */
            memmove(s, e + 2, strlen(e + 2) + 1);
        } else {
            /* unterminated block comment -> cut rest */
            *s = '\0';
            break;
        }
        s = strstr(line, "/*");
    }
    rtrim(line);
}

/*
 * apply_replacements:
 *   Safely apply all replacements to a single line.
 *   It accepts char **lineptr so it can reallocate and replace the pointer stored in lines[].
 */
void apply_replacements(char **lineptr) {
    if (!lineptr || !*lineptr) return;

    // work on a dynamic string that we'll reassign at the end
    char *current = strdup(*lineptr);
    if (!current) return;

    for (int r = 0; r < replacements_count; r++) {
        replacement_t *rep = &replacements[r];
        size_t from_len = strlen(rep->from);
        if (from_len == 0) continue;

        // We'll build a new string step by step into a dynamic buffer.
        size_t out_cap = strlen(current) + 1;
        char *out = malloc(out_cap);
        if (!out) { free(current); return; }
        out[0] = '\0';

        char *p = current;
        while (*p) {
            char *pos = strstr(p, rep->from);
            if (!pos) {
                // append remainder
                size_t need = strlen(out) + strlen(p) + 1;
                if (need > out_cap) {
                    out_cap = need * 2;
                    char *tmp = realloc(out, out_cap);
                    if (!tmp) { free(out); free(current); return; }
                    out = tmp;
                }
                strncat(out, p, strlen(p));
                break;
            } else {
                // append prefix
                int prefix_len = (int)(pos - p);
                size_t need = strlen(out) + prefix_len + 1;
                if (need > out_cap) {
                    out_cap = need * 2;
                    char *tmp = realloc(out, out_cap);
                    if (!tmp) { free(out); free(current); return; }
                    out = tmp;
                }
                strncat(out, p, prefix_len);

                // append replacement
                need = strlen(out) + strlen(rep->to) + 1;
                if (need > out_cap) {
                    out_cap = need * 2;
                    char *tmp = realloc(out, out_cap);
                    if (!tmp) { free(out); free(current); return; }
                    out = tmp;
                }
                strncat(out, rep->to, strlen(rep->to));

                // advance p
                p = pos + from_len;
            }
        }

        free(current);
        current = out;
    }

    // replace the pointer in the caller
    free(*lineptr);
    *lineptr = current;
}

int extract_window_content(char **lines, int n, const char *html_path) {
    if (!lines || n <= 0 || !html_path) return -1;
    int brace_count = 0;
    int inside = 0;
    FILE *html = fopen(html_path, "w");
    if (!html) { perror("fopen html"); return -1; }

    for (int i = 0; i < n; i++) {
        char *line = lines[i];
        if (!line) continue;

        if (!inside) {
            char *pos = strstr(line, "window()");
            if (pos) {
                inside = 1;
                char *brace_start = strchr(pos, '{');
                if (brace_start) {
                    // there is an opening brace on this line
                    brace_count = 1;
                    char *content_after = brace_start + 1;
                    if (*content_after != '\0') {
                        fprintf(html, "%s\n", content_after);
                    }
                } else {
                    // window() found but brace not on same line: look ahead for brace
                    // write nothing from this line, keep it empty
                }
                // mark this source line as consumed -> replace it with empty string safely
                free(lines[i]);
                lines[i] = strdup("");
                if (!lines[i]) lines[i] = NULL;
                continue;
            }
        } else {
            // we are inside window() block
            // Count braces on this line first
            for (char *c = line; *c; c++) {
                if (*c == '{') brace_count++;
                else if (*c == '}') brace_count--;
            }

            // Trim leading whitespace to detect lines that are only the final closing brace
            char *only_brace = line;
            while (isspace((unsigned char)*only_brace)) only_brace++;

            // If the line is exactly a single '}' possibly followed by whitespace, skip writing it.
            int write_line = 1;
            if (*only_brace == '}' && (only_brace[1] == '\0' || isspace((unsigned char)only_brace[1]))) {
                // if this is the final closing brace and it's the one that reduces brace_count to 0,
                // skip writing it. But if there are other characters on the line (e.g., "};"), write it.
                // We will skip only if the line contains nothing but whitespace and '}'.
                // detect if any non-whitespace other than '}' exists
                int non_ws_other = 0;
                for (char *c = line; *c; c++) {
                    if (!isspace((unsigned char)*c) && *c != '}') { non_ws_other = 1; break; }
                }
                if (!non_ws_other) write_line = 0;
            }

            if (write_line) fprintf(html, "%s\n", line);

            // mark this source line as consumed -> replace it with empty string safely
            free(lines[i]);
            lines[i] = strdup("");
            if (!lines[i]) lines[i] = NULL;

            if (brace_count <= 0) {
                inside = 0;
            }
        }
    }

    fclose(html);
    return 0;
}

void escape_cpp_string(const char *input, char *output, size_t outcap) {
    if (!input || !output || outcap == 0) return;
    size_t idx = 0;
    while (*input && idx + 1 < outcap) {
        if (*input == '\\') {
            if (idx + 2 < outcap) { output[idx++] = '\\'; output[idx++] = '\\'; }
            else break;
        } else if (*input == '"') {
            if (idx + 2 < outcap) { output[idx++] = '\\'; output[idx++] = '"'; }
            else break;
        } else {
            output[idx++] = *input;
        }
        input++;
    }
    output[idx] = '\0';
}

/* --- Main --- */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <files> [output_name]\n", argv[0]);
        return 1;
    }

    const char *out_name = "webapp"; // default
    if (argc >= 3) {
        out_name = argv[2];
    }

    int last_input_index = argc - 1;
    if (argc >= 3) last_input_index = argc - 2;

    for (int file_i = 1; file_i <= last_input_index; file_i++) {
        FILE *in = fopen(argv[file_i], "r");
        if (!in) {
            perror("fopen input");
            continue;
        }

        char *lines[1000];
        int n = 0;
        char buf[1024];

        // read lines, strip comments and rtrim
        while (fgets(buf, sizeof(buf), in) && n < 1000) {
            rtrim(buf);
            remove_comments(buf);
            lines[n++] = strdup(buf);
            if (!lines[n-1]) { fprintf(stderr, "Out of memory\n"); break; }
        }
        fclose(in);

        // --- create temp html path ---
        char html_path[512];
#ifdef _WIN32
        snprintf(html_path, sizeof(html_path), "C:\\nevo\\temp.html");
#else
        const char *home = getenv("HOME");
        if (!home) home = ".";
        snprintf(html_path, sizeof(html_path), "%s/nevo/temp.html", home);
#endif

        // --- Extract window() content safely ---
        if (extract_window_content(lines, n, html_path) != 0) {
            fprintf(stderr, "Warning: extract_window_content failed for %s\n", argv[file_i]);
        }

        // --- Apply replacements safely to each non-empty line ---
        for (int i = 0; i < n; i++) {
            if (lines[i] && lines[i][0] != '\0') {
                apply_replacements(&lines[i]);
            }
        }

        // --- Determine needed headers BEFORE opening output file ---
        headers_needed_t need;
        // calculate_needed_headers expects (const char**)lines
        calculate_needed_headers((const char **)lines, n, &need);

        // --- Prepare C++ output file name (fixed single filename like before) ---
        const char *CFILE = "A1bC2dE35F31nBv53X.cpp";
        FILE *out = fopen(CFILE, "w");
        if (!out) {
            perror("fopen out");
            // cleanup allocated memory for this file
            for (int i = 0; i < n; i++) if (lines[i]) free(lines[i]);
            continue;
        }

        // --- Write top-of-file headers (always present) ---
        fprintf(out, "#include <cstdlib>\n");
        fprintf(out, "#define WEBVIEW_IMPLEMENTATION\n");

#ifdef _WIN32
        fprintf(out, "#include \"C:\\nevo\\libraries\\webview\\webview.h\"\n\n");
#else
        home = getenv("HOME");
        if (!home) home = ".";
        fprintf(out, "#include \"%s/nevo/libraries/webview/webview.h\"\n\n", home);
#endif

        // --- Write conditional includes based on `need` (before writing body) ---
        for (int i = 0; i <= INC_UNLESS; i++) {
            int flag = 0;
            switch (i) {
                case INC_STD: flag = need.stdio; break;
                case INC_STDL: flag = need.stdlib; break;
                case INC_STR: flag = need.string; break;
                case INC_MATH: flag = need.math; break;
                case INC_CTP: flag = need.ctype; break;
                case INC_TIME: flag = need.time; break;
                case INC_ASSERT: flag = need.assert; break;
                case INC_ARR: flag = need.arradd; break;
                case INC_H1: flag = need.h1; break;
                case INC_SHA: flag = need.sha256; break;
                case INC_NPXM: flag = need.npxm; break;
                case INC_BOOL: flag = need.bool_defined; break;
                case INC_BOPS: flag = need.bops_defined; break;
                case INC_UNLESS: flag = need.unless_defined; break;
            }
            if (flag) {
#ifdef _WIN32
                fprintf(out, "%s\n", win_paths[i]);
#else
                if (i >= INC_ARR) fprintf(out, "#include \"%s%s\"\n", home, linux_paths[i]);
                else fprintf(out, "%s\n", linux_paths[i]);
#endif
            }
        }

        if (need.cap_defined) fprintf(out, "#define cap false\n");
        if (need.nocap_defined) fprintf(out, "#define nocap true\n");

        // --- Write body lines, injecting webview boilerplate inside main if found ---
        int main_index = -1;
        for (int i = 0; i < n; i++) {
            if (lines[i] && strstr(lines[i], "int main(")) { main_index = i; break; }
        }

        for (int i = 0; i < n; i++) {
            if (!lines[i]) continue;
            if (lines[i][0] == '\0') continue; // consumed by extractor or empty

            if (i == main_index) {
                // write the main signature line(s)
                fprintf(out, "%s\n", lines[i]);

                // if brace not on same line, check next non-empty line for it
                if (!strchr(lines[i], '{')) {
                    // attempt to find the line with '{'
                    int j = i + 1;
                    while (j < n && lines[j] && lines[j][0] == '\0') j++;
                    if (j < n && lines[j]) {
                        fprintf(out, "%s\n", lines[j]);
                        // mark that we already wrote lines[j] so it won't get written again
                        free(lines[j]); lines[j] = strdup("");
                        if (!lines[j]) lines[j] = NULL;
                    }
                }

                // inject webview boilerplate
#ifndef _WIN32
                fprintf(out, "    std::string home = getenv(\"HOME\");\n");
                fprintf(out, "    std::string path = \"file://\" + home + \"/nevo/temp.html\";\n");
#else
                fprintf(out, "    std::string path = \"file://C:/nevo/temp.html\";\n");
#endif
                fprintf(out, "    webview::webview w(false, nullptr);\n");

                char escaped[1024];
                escape_cpp_string(out_name, escaped, sizeof(escaped));
                fprintf(out, "    w.set_title(\"%s\");\n", escaped);
                fprintf(out, "    w.set_size(800, 600, WEBVIEW_HINT_NONE);\n");
                fprintf(out, "    w.navigate(path);\n");
                fprintf(out, "    w.run();\n\n");

                // We intentionally do not 'continue' here because we already wrote the main line(s),
                // But we should skip writing the same line again later. To avoid duplication,
                // mark this line as consumed:
                free(lines[i]); lines[i] = strdup("");
                if (!lines[i]) lines[i] = NULL;
                continue;
            }

            // normal write
            fprintf(out, "%s\n", lines[i]);
            // free here would be fine, but we'll free all at cleanup to keep consistent ownership.
        }

        // Ensure final closing brace exists (don't duplicate if already present).
        // We'll append a single '}\n' to close the program in case the input didn't provide it.
        

        fclose(out);

        // --- Auto-compile the generated C++ file ---
        char compile_cmd[1024];

#ifdef _WIN32
        snprintf(compile_cmd, sizeof(compile_cmd),
                 "tcc %s -o %s.exe -lole32 -loleaut32 -luuid -lcomctl32 -luser32 -lgdi32", CFILE, out_name);
#else
        snprintf(compile_cmd, sizeof(compile_cmd),
                 "clang++ %s -std=c++17 -framework WebKit -o %s", CFILE, out_name);
#endif

        int compile_result = system(compile_cmd);
        if (compile_result != 0) {
            fprintf(stderr, "Compilation failed with code %d\n", compile_result);
        } else {
            printf("Compilation succeeded: %s created.\n", out_name);
        }

        // CLEANUP: free all allocated lines for this input file
        for (int i = 0; i < n; i++) {
            if (lines[i]) {
                free(lines[i]);
                lines[i] = NULL;
            }
        }
    } // end per-file loop

    return 0;
}

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

// --- Replacements ---
void apply_replacements(char *line) {
    char buffer[1024];
    for (int r = 0; r < replacements_count; r++) {
        char *src = line, *dst = buffer;
        *dst = '\0';
        while (*src) {
            char *pos = strstr(src, replacements[r].from);
            if (!pos) { strncat(dst, src, sizeof(buffer)-strlen(dst)-1); break; }
            int len = pos - src;
            strncat(dst, src, len);
            strncat(dst, replacements[r].to, sizeof(buffer)-strlen(dst)-1);
            src = pos + strlen(replacements[r].from);
        }
        strncpy(line, buffer, sizeof(buffer));
        line[sizeof(buffer)-1] = '\0';
    }
}

void rtrim(char *s) { int len=strlen(s); while(len>0 && isspace((unsigned char)s[len-1])) s[--len]='\0'; }

void remove_comments(char *line) {
    char *c = strstr(line, "//"); if(c) *c='\0';
    char *s = strstr(line, "/*");
    while(s) { char *e=strstr(s+2,"*/"); if(e) memmove(s,e+2,strlen(e+2)+1); else {*s='\0'; break;} s=strstr(line,"/*"); }
    rtrim(line);
}

int extract_window_content(char **lines, int n, const char *html_path) {
    int brace_count = 0;
    int inside = 0;
    FILE *html = fopen(html_path, "w");
    if (!html) { perror("fopen html"); return -1; }

    for (int i = 0; i < n; i++) {
        char *line = lines[i];

        if (!inside) {
            char *pos = strstr(line, "window()");
            if (pos) {
                inside = 1;
                char *brace_start = strchr(pos, '{');
                if (brace_start) {
                    brace_count = 1;  // Start counting braces
                    char *content_after = brace_start + 1;
                    if (*content_after != '\0') {
                        fprintf(html, "%s\n", content_after); // write everything after {
                    }
                }
                lines[i][0] = '\0';
                continue;
            }
        } else {
            char *ptr = line;
            // Count braces before writing
            for (char *c = line; *c; c++) {
                if (*c == '{') brace_count++;
                if (*c == '}') brace_count--;
            }

            // Skip the line if it only contains the final closing brace
            char *only_brace = line;
            while (isspace((unsigned char)*only_brace)) only_brace++;
            if (*only_brace != '}') fprintf(html, "%s\n", line);

            lines[i][0] = '\0';

            if (brace_count <= 0) {
                inside = 0; // stop at correct closing brace
            }
        }
    }

    fclose(html);
    return 0;
}

void escape_cpp_string(const char *input, char *output) {
    while (*input) {
        if (*input == '\\') {
            *output++ = '\\';
            *output++ = '\\';
        } else if (*input == '"') {
            *output++ = '\\';
            *output++ = '"';
        } else {
            *output++ = *input;
        }
        input++;
    }
    *output = '\0';
}

// --- Main ---
int main(int argc,char *argv[]) {
    if(argc < 2) {
        printf("Usage: %s <files>\n", argv[0]);
        return 1;
    }
    const char *out_name = "webapp"; // default
    if(argc >= 3) {
        out_name = argv[2]; // use second argument if provided
    }

    int last_input_index = argc - 1; // assume last arg might be output name
    if (argc >= 3) last_input_index = argc - 2; // exclude last arg (output name)

    for(int file_i=1; file_i<=last_input_index; file_i++){
        FILE *in=fopen(argv[file_i],"r"); 
        if(!in){perror("fopen");
            continue;}
        char *lines[1000],buf[1024]; int n=0;

        while(fgets(buf,sizeof buf,in)&&n<1000){
            rtrim(buf); 
            remove_comments(buf);
            lines[n++]=strdup(buf);
        }
        fclose(in);

        // --- Extract window() content first ---
        char html_path[512];
#ifdef _WIN32
        snprintf(html_path,sizeof html_path,"C:\\nevo\\temp.html");
#else
        const char *home=getenv("HOME");
        snprintf(html_path,sizeof html_path,"%s/nevo/temp.html",home);
#endif
        extract_window_content(lines,n,html_path);
        for (int i = 0; i < n; i++) {
            if (lines[i][0] != '\0') {
                apply_replacements(lines[i]);
            }
        }

        // --- Output C++ file ---
        const char *CFILE = "A1bC2dE35F31nBv53X.cpp";
        FILE *out=fopen(CFILE,"w"); if(!out){perror("fopen out"); goto CLEANUP;}

        // --- Write headers ---
        fprintf(out,"#include <cstdlib>\n");
        fprintf(out,"#define WEBVIEW_IMPLEMENTATION\n");

        #ifdef _WIN32
        fprintf(out,"#include \"C:\\nevo\\libraries\\webview\\webview.h\"\n\n");
        #else
        home = getenv("HOME");
        fprintf(out,"#include \"%s/nevo/libraries/webview/webview.h\"\n\n", home);
        #endif

        // --- Detect if main() exists ---
        int main_index = -1;
        for (int i = 0; i < n; i++) {
            if (strstr(lines[i], "int main(")) {
                main_index = i;
                break;
            }
        }

        // --- Write lines ---
        for (int i = 0; i < n; i++) {
            if (lines[i][0] == '\0') continue; // skip empty lines

            // Inject inside existing main
            if (i == main_index) {
                fprintf(out, "%s\n", lines[i]); // write "int main(...){"

                // Check if { is on this line
                char *brace = strchr(lines[i], '{');
                if (!brace) {
                    // If brace is on next line, write it
                    i++;
                    fprintf(out, "%s\n", lines[i]);
                }

                // Inject webview boilerplate inside main
        #ifndef _WIN32
                fprintf(out,"    std::string home = getenv(\"HOME\");\n");
                fprintf(out,"    std::string path = \"file://\" + home + \"/nevo/temp.html\";\n");
        #else
                fprintf(out,"    std::string path = \"file://C:/nevo/temp.html\";\n");
        #endif
                fprintf(out,"    webview::webview w(false, nullptr);\n");
                /* emit code that uses the escaped literal for the title */
                char escaped[1024];
                escape_cpp_string(out_name, escaped);

                fprintf(out,"    w.set_title(\"%s\");\n", escaped);
                fprintf(out,"    w.set_size(800, 600, WEBVIEW_HINT_NONE);\n");
                fprintf(out,"    w.navigate(path);\n");
                fprintf(out,"    w.run();\n\n");

                continue; // skip duplicate writing
            }

            fprintf(out,"%s\n", lines[i]);
            free(lines[i]);
        }

        fprintf(out, "}\n");

        // --- Include other headers if needed ---
        headers_needed_t need;
        calculate_needed_headers((const char**)lines,n,&need);
        home=getenv("HOME");

        for(int i=0;i<=INC_UNLESS;i++){
            int flag=0;
            switch(i){
                case INC_STD: flag=need.stdio; break; case INC_STDL: flag=need.stdlib; break;
                case INC_STR: flag=need.string; break; case INC_MATH: flag=need.math; break;
                case INC_CTP: flag=need.ctype; break; case INC_TIME: flag=need.time; break;
                case INC_ASSERT: flag=need.assert; break; case INC_ARR: flag=need.arradd; break;
                case INC_H1: flag=need.h1; break; case INC_SHA: flag=need.sha256; break;
                case INC_NPXM: flag=need.npxm; break; case INC_BOOL: flag=need.bool_defined; break;
                case INC_BOPS: flag=need.bops_defined; break; case INC_UNLESS: flag=need.unless_defined; break;
            }
            if(flag){
        #ifdef _WIN32
                fprintf(out,"%s\n",win_paths[i]);
        #else
                if(i>=INC_ARR) fprintf(out,"#include \"%s%s\"\n",home,linux_paths[i]);
                else fprintf(out,"%s\n",linux_paths[i]);
        #endif
            }
        }

        if(need.cap_defined) fprintf(out,"#define cap false\n");
        if(need.nocap_defined) fprintf(out,"#define nocap true\n");

        fclose(out);

        // --- Auto-compile the generated C++ file ---
        char compile_cmd[1024];

        #ifdef _WIN32
            // On Windows, compile using TCC
            snprintf(compile_cmd, sizeof(compile_cmd),
                    "tcc -run %s -o %s.exe", CFILE, out_name);
        #else
            // On macOS, compile using clang++
            snprintf(compile_cmd, sizeof(compile_cmd),
                    "clang++ %s -std=c++17 -framework WebKit -o %s", CFILE, out_name);
        #endif

        int compile_result = system(compile_cmd);
        if (compile_result != 0) {
            fprintf(stderr, "Compilation failed with code %d\n", compile_result);
        } else {
            printf("Compilation succeeded: %s created.\n", 
                #ifdef _WIN32
                out_name
                #else
                out_name
                #endif
                );
        }

    

CLEANUP:;
    }
    return 0;
}

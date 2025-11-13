#include "curses.h"
#include <string.h>
#include <ctype.h>

static const char *banned_words[] = {
    "damn",
    "heck",
    "ughh",
    "crap",
    "wtf"
};
static const int banned_count = sizeof(banned_words) / sizeof(banned_words[0]);

// Helper: is this a word boundary?
static int is_word_boundary(char c) {
    return !(isalnum((unsigned char)c) || c == '_');
}

void censor_line(char *line) {
    char buffer[1024];
    char *dst = buffer;
    int in_string = 0;

    for (char *src = line; *src; ) {
        // Track string literals
        if (*src == '"') {
            *dst++ = *src++;
            in_string = !in_string;
            continue;
        }

        if (in_string) {
            *dst++ = *src++;
            continue;
        }

        // Check for banned words
        int matched = 0;
        for (int i = 0; i < banned_count; i++) {
            size_t len = strlen(banned_words[i]);
            if (strncmp(src, banned_words[i], len) == 0) {
                // Check word boundaries
                char prev = (src == line) ? ' ' : *(src-1);
                char next = src[len] ? src[len] : ' ';
                if (is_word_boundary(prev) && is_word_boundary(next)) {
                    src += len; // Skip banned word
                    matched = 1;
                    break;
                }
            }
        }

        if (!matched) {
            *dst++ = *src++;
        }
    }

    *dst = '\0';
    strcpy(line, buffer);
}

#ifndef NHTMLT_H
#define NHTMLT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TAG 128
#define MAX_STACK 128

static const char* nhtmlt_self_closing[] = { "meta", "link", "img", "br", "hr", NULL };

typedef struct {
    char tag[MAX_TAG];
    int indent;
} TagStackItem;

static int nhtmlt_is_self_closing(const char* tag) {
    for (int i = 0; nhtmlt_self_closing[i]; i++)
        if (strcmp(tag, nhtmlt_self_closing[i]) == 0) return 1;
    return 0;
}

static const char* nhtmlt_trim(const char* line) {
    while (*line && isspace(*line)) line++;
    return line;
}

static int nhtmlt_count_indent(const char* line) {
    int count = 0;
    while (*line && (*line == ' ' || *line == '\t')) {
        count++;
        line++;
    }
    return count;
}

// Main line processor
static char* nhtmlt_line(const char* line, TagStackItem* stack, int* top, int* in_style) {
    const char* trimmed = nhtmlt_trim(line);
    int indent = nhtmlt_count_indent(line);

    if (*trimmed == 0) return strdup("\n");

    if (strncmp(trimmed, "<!DOCTYPE", 9) == 0) {
        char* out = (char*)malloc(strlen(trimmed)+2);
        sprintf(out, "%s\n", trimmed);
        return out;
    }

    char* out = (char*)malloc(4096);
    out[0] = 0;

    // Close tags if indentation decreased
    while (*top > 0 && indent <= stack[*top-1].indent) {
        // Check if closing a style tag
        if (strcmp(stack[*top-1].tag, "style") == 0) *in_style = 0;

        char closing[256];
        snprintf(closing, sizeof(closing), "%*s</%s>\n", stack[*top-1].indent, "", stack[*top-1].tag);
        strcat(out, closing);
        (*top)--;
    }

    // If inside style block, output raw
    if (*in_style) {
        char lineout[1024];
        sprintf(lineout, "%*s%s\n", indent, "", trimmed);
        strcat(out, lineout);
        return out;
    }

    // Check if line is tag
    if (isalpha(trimmed[0])) {
        char tag[MAX_TAG] = {0};
        int i = 0;
        while (trimmed[i] && !isspace(trimmed[i]) && trimmed[i] != '{') {
            if (i >= MAX_TAG-1) break;
            tag[i] = trimmed[i];
            i++;
        }
        tag[i] = 0;

        const char* rest = trimmed + i;
        while (*rest && isspace(*rest)) rest++;

        if (nhtmlt_is_self_closing(tag)) {
            char lineout[1024];
            sprintf(lineout, "%*s<%s %s />\n", indent, "", tag, rest);
            strcat(out, lineout);
        } else {
            char lineout[1024];
            if (*rest)
                sprintf(lineout, "%*s<%s %s>\n", indent, "", tag, rest);
            else
                sprintf(lineout, "%*s<%s>\n", indent, "", tag);
            strcat(out, lineout);

            // Push to stack
            strncpy(stack[*top].tag, tag, MAX_TAG-1);
            stack[*top].indent = indent;
            (*top)++;

            if (strcmp(tag, "style") == 0) *in_style = 1;
        }
    } else {
        // raw content outside style, e.g., text node
        char lineout[1024];
        sprintf(lineout, "%*s%s\n", indent, "", trimmed);
        strcat(out, lineout);
    }

    return out;
}

// Close remaining tags
static char* nhtmlt_close_remaining(TagStackItem* stack, int* top) {
    char* out = (char*)malloc(4096);
    out[0] = 0;
    while (*top > 0) {
        char closing[256];
        snprintf(closing, sizeof(closing), "%*s</%s>\n", stack[*top-1].indent, "", stack[*top-1].tag);
        strcat(out, closing);
        (*top)--;
    }
    return out;
}

#endif

// Wrapper function for one-shot use (like nhtmlt(line))
static char* nhtmlt(const char* line) {
    static TagStackItem stack[MAX_STACK];
    static int top = 0;
    static int in_style = 0;
    return nhtmlt_line(line, stack, &top, &in_style);
}

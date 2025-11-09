#ifndef BOOL_H
#define BOOL_H

typedef enum {
    TYPE_FALSE = 0,
    TYPE_TRUE = 1,
    TYPE_MAYBE = 2,
    TYPE_SOMETIMES = 3,
    TYPE_REPEAT = 4  // "can you repeat the question?"
} type;

// --- Core logic operators ---
type type_and(type a, type b);
type type_or(type a, type b);
type type_not(type a);

// --- Utility ---
int type_truthy(type v);
const char* type_to_string(type v);

#endif

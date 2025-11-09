#include "bool.h"
#include <stdio.h>

type type_and(type a, type b) {
    if (a == TYPE_FALSE || b == TYPE_FALSE) return TYPE_FALSE;
    if (a == TYPE_REPEAT || b == TYPE_REPEAT) return TYPE_REPEAT;
    if (a == TYPE_MAYBE || b == TYPE_MAYBE) return TYPE_MAYBE;
    if (a == TYPE_SOMETIMES || b == TYPE_SOMETIMES) return TYPE_SOMETIMES;
    return TYPE_TRUE;
}

type type_or(type a, type b) {
    if (a == TYPE_TRUE || b == TYPE_TRUE) return TYPE_TRUE;
    if (a == TYPE_REPEAT || b == TYPE_REPEAT) return TYPE_REPEAT;
    if (a == TYPE_SOMETIMES || b == TYPE_SOMETIMES) return TYPE_SOMETIMES;
    if (a == TYPE_MAYBE || b == TYPE_MAYBE) return TYPE_MAYBE;
    return TYPE_FALSE;
}

type type_not(type a) {
    switch (a) {
        case TYPE_TRUE: return TYPE_FALSE;
        case TYPE_FALSE: return TYPE_TRUE;
        case TYPE_MAYBE: return TYPE_MAYBE;
        case TYPE_SOMETIMES: return TYPE_SOMETIMES;
        case TYPE_REPEAT: return TYPE_REPEAT;
        default: return TYPE_REPEAT;
    }
}

int type_truthy(type v) {
    return (v == TYPE_TRUE || v == TYPE_SOMETIMES);
}

const char* type_to_string(type v) {
    switch (v) {
        case TYPE_TRUE: return "true";
        case TYPE_FALSE: return "false";
        case TYPE_MAYBE: return "maybe";
        case TYPE_SOMETIMES: return "sometimes";
        case TYPE_REPEAT: return "can you repeat the question?";
        default: return "unknown";
    }
}

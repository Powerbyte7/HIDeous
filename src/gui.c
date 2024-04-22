
#include "gui.h"

static const char* keynames1[] = {
    "0",
    "ENTER",
    "ESC",
    "BACKSPACE",
    "TAB",
    "SPACE",
    "MINUS",
    "EQUAL",
    "LEFTBRACE",
    "RIGHTBRACE",
    "BACKSLASH",
    "HASHTILDE",
    "SEMICOLON",
    "APOSTROPHE",
    "GRAVE",
    "COMMA",
    "DOT",
    "SLASH",
    "CAPSLOCK"
};

static const char* keynames2[] = {
    "SYSRQ",
    "SCROLLLOCK",
    "PAUSE",
    "INSERT",
    "HOME",
    "PAGEUP ",
    "DELETE",
    "END",
    "PAGEDOWN",
    "RIGHT",
    "LEFT",
    "DOWN",
    "UP",
    "NUMLOCK",
    "KPSLASH",
    "KPASTERISK",
    "KPMINUS",
    "KPPLUS",
    "KPENTER"
};

static const char* keynames3[] = {
    "KP0",
    "KPDOT",
    "102ND",
    "COMPOSE",
    "POWER",
    "KPEQUAL" 
};

static const char* keynames4[] = {
    "CTRL",
    "SHIFT",
    "ALT",
    "META"
};

void printKey(uint8_t key) {
    os_FontSelect(os_LargeFont);
    if (key < KEY_A) {
        printf("NONE\n");
        return;
    }

    printf("KEY_");

    if (key <= KEY_Z) {
        printf("%c", key+('A'-KEY_A));
    } else if (key <= KEY_9) {
        printf("%d", key-KEY_Z);
    } else if (key <= KEY_CAPSLOCK) {
        printf("%s", keynames1[key-KEY_0]);
    } else if (key <= KEY_F12) {
        printf("F%d", key-KEY_CAPSLOCK);
    } else if (key <= KEY_KPENTER) {
        printf("%s", keynames2[key-KEY_SYSRQ]);
    } else if (key <= KEY_KP9) {
        printf("KP%d", key-KEY_KPENTER);
    } else if (key <= KEY_KPEQUAL) {
        printf("%s", keynames3[key-KEY_KP0]);
    } else if (key <= KEY_F24) {
        printf("F%d", key-KEY_KPEQUAL);
    } else if (key >= KEY_LEFTCTRL && key <= KEY_LEFTMETA) {
        printf("%s", keynames4[key-KEY_LEFTCTRL]);
    } else {
        printf("OTHER");
    }

    printf("\n");
}
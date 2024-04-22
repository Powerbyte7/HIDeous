
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

// Prints the names of most HID keys
void printKey(uint8_t key) {
    if (key < KEY_A) {
        os_PutStrFull("NONE");
        os_NewLine();
        return;
    }

    os_PutStrFull("KEY_");

    if (key <= KEY_Z) {
        char str[2] = {('A'-KEY_A), '\0'};
        str[0] += key;
        os_PutStrFull(str);
    } else if (key <= KEY_9) {
        char str[2];
        sprintf(str, "%u", key-KEY_Z);
        os_PutStrFull(str);
    } else if (key <= KEY_CAPSLOCK) {
        os_PutStrFull(keynames1[key-KEY_0]);
    } else if (key <= KEY_F12) {
        char str[4];
        sprintf(str, "F%u", key-KEY_CAPSLOCK);
        os_PutStrFull(str);
    } else if (key <= KEY_KPENTER) {
        os_PutStrFull(keynames2[key-KEY_SYSRQ]);
    } else if (key <= KEY_KP9) {
        char str[4];
        sprintf(str, "KP%u", key-KEY_KPENTER);
        os_PutStrFull(str);
    } else if (key <= KEY_KPEQUAL) {
        os_PutStrFull(keynames3[key-KEY_KP0]);
    } else if (key <= KEY_F24) {
        char str[4];
        sprintf(str, "F%u", key-KEY_KPEQUAL);
        os_PutStrFull(str);
    } else if (key >= KEY_LEFTCTRL && key <= KEY_LEFTMETA) {
        os_PutStrFull(keynames4[key-KEY_LEFTCTRL]);
    } else {
        os_PutStrFull("OTHER");
    }

    os_NewLine();
}
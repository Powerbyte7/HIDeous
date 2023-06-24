# ----------------------------
# Makefile Options
# ----------------------------

NAME = HIDeous
ICON = icon.png
DESCRIPTION = "HID keyboard emulator"
COMPRESSED = NO
ARCHIVED = NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)

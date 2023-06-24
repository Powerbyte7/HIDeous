# ----------------------------
# Makefile Options
# ----------------------------

NAME = HID4
ICON = icon.png
DESCRIPTION = "HID emulator"
COMPRESSED = NO
ARCHIVED = NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)

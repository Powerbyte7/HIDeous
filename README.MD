# HIDeous
This program turns the Ti-84 CE into a working keyboard by emulating a HID device over USB. No extra software required!

## Installation
1. Install TI-Connect CE.
2. Connect your calculator to your computer.
3. Send [clibs.8xg](https://github.com/CE-Programming/libraries/releases/tag/v11.1) to your calculator
4. Send HIDeous.8xp to your calculator.

## Usage

<p align="center">
    <img src="https://raw.githubusercontent.com/Powerbyte7/HIDeous/master/keyguide.svg?sanitize=true"
        width="800">
</p>

## Changing the key layout

To change the key layout you'll need to compile HIDeous yourself. Don't worry, it's a fairly straightfoward process. 

1. [Install the CE toolchain](https://ce-programming.github.io/toolchain/static/getting-started.html)
2. Download the [source code](https://github.com/Powerbyte7/HIDeous/archive/refs/heads/master.zip)
3. In hid.c, change the keycodes in the map[] and/or special_map[] arrays. You can find the keycodes in usb_hid_keys.h
4. Build the program and send the resulting 8xp file to your calculator.

## Adding Macros

To add macros you'll need to replace the macro AppVars (HIDM1-HIDM5) with your own macro sequence. You can again use all of the keys defined in usb_hid_keys.h. By defaut, HIDeous will toggle the key twice in a short timeframe. If you want to toggle a key once to keep it pressed down, you need to preface the key with KEY_ONCE.
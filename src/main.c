#include <usbdrvce.h>
#include <stdio.h>
#include <stdlib.h>
#include <tice.h>
#include <string.h>
#include <keypadc.h>
#include <fileioc.h>

#include "main.h"
#include "keymap.h"
#include "macros.h"
#include "gui.h"
#include "usb_hid_keys.h"

#define DEFAULT_LANGID 0x0409
#define MACRO_DELAY 0x016F
#define KEY_ONCE 0x03

// Toggles keys in last 6 bytes in input_data
static uint8_t toggle_hid_key(uint8_t key, uint8_t input_data[]) {

    static uint8_t prev_key;

    // Toggles the modifier bits in the first byte of input_data
    switch (key) {
        case KEY_LEFTSHIFT:
            input_data[0] = input_data[0] ^ SHIFT_BIT;
            return 0;
        case KEY_LEFTCTRL:
            input_data[0] = input_data[0] ^ CTRL_BIT;
            return 0;
        case KEY_LEFTALT:
            input_data[0] = input_data[0] ^ ALT_BIT;
            return 0;
        case KEY_LEFTMETA:
            input_data[0] = input_data[0] ^ GUI_BIT;
            break; // Doesn't return because GUI key needs to be set as byte too
    }

    // If key already exists, set to KEY_NONE
    for (int i = 2; i <= 7; i++) {
        prev_key = input_data[i];

        if (prev_key == key) {
            input_data[i] = KEY_NONE;
            return 0;
        }
    }

    // If key doesn't exist, add it to first available slot
    for (int i = 2; i <= 7; i++) {
       prev_key = input_data[i];

        if (prev_key == KEY_NONE) {
            input_data[i] = key;
            printKey(key);
            return 0;
        }
    }

    // If there's no space left in the array, send overflow packet
    // Happens when more than 6 keys are pressed at once
    return ROLLOVER_ERR; 
}

// Adds keys to the 8-byte HID input_data
static uint8_t toggle_sk_key(uint8_t key, uint8_t input_data[]) {
    // Toggle for normal/special characters
    static bool special_mode;

    if (key == sk_Mode) {
        special_mode = !special_mode;
    }

    // GSC key conversion to HID key
    uint8_t hid_key;
    if (!special_mode) {
        hid_key = key < sizeof(map) / sizeof(*map) ? map[key] : KEY_NONE;
    } else {
        hid_key = key < sizeof(special_map) / sizeof(*special_map) ? special_map[key] : KEY_NONE;
    }

    return hid_key ? toggle_hid_key(hid_key, input_data) : 0; 
}

static usb_device_t active_device;

// int debug_printed = 0;
// void temp(void* array, void* structure) {
//     if (debug_printed) {
//         return;
//     }

//     debug_printed = 1;
//     printf("\n");
//     for (int i = 0; i<sizeof(usb_control_setup_t); i+=4) {
//         printf("USB: %02X %02X %02X %02X\n", (char*)*((char*)structure+i), (char*)*((char*)structure+1+i), (char*)*((char*)structure+2+i), (char*)*((char*)structure+3+i));
//         printf("VAR: %02X %02X %02X %02X\n", (char*)*((char*)array+i), (char*)*((char*)array+1+i), (char*)*((char*)array+2+i), (char*)*((char*)array+3+i));
//     }

//     if (!memcmp(array, structure, sizeof(usb_control_setup_t))) {
//         printf("It works\n");
//     } else {
//         printf("It does not\n");
//     }
    
// }

static usb_error_t handleUsbEvent(usb_event_t event, void *event_data,
                                  void *unused) {

    (void)unused;

    static const uint8_t hid_report_check[8] = {
        0x81,
        0x6,
        0x0,
        0x22,
        0x0,
        0x0,
        0x7F
    };

    static const uint8_t hid_report_descriptor[63] = {
        5, 0x1,      // USAGE_PAGE (Generic Desktop)
        9, 0x6,      // USAGE (Keyboard)
        0x0A1, 0x1,  // COLLECTION (Application)
        0x5, 7,      //   USAGE_PAGE (Keyboard)
        0x19, 0x0E0, //   USAGE_MINIMUM (Keyboard LeftControl)
        0x29, 0x0E7, //   USAGE_MAXIMUM (Keyboard Right GUI)
        0x15, 0x0,   //   LOGICAL_MINIMUM (0)
        0x25, 0x1,   //   LOGICAL_MAXIMUM (1)
        0x75, 0x1,   //   REPORT_SIZE (1)
        0x95, 0x8,   //   REPORT_COUNT (8)
        0x81, 0x2,   //   INPUT (Data,Var,Abs)
        0x95, 0x1,   //   REPORT_COUNT (1)
        0x75, 0x8,   //   REPORT_SIZE (8)
        0x81, 0x3,   //   INPUT (Cnst,Var,Abs) 81 03
        0x95, 0x5,   //   REPORT_COUNT (5)
        0x75, 0x1,   //   REPORT_SIZE (1)
        0x5, 0x8,    //   USAGE_PAGE (LEDs)
        0x19, 0x1,   //   USAGE_MINIMUM (Num Lock)
        0x29, 0x5,   //   USAGE_MAXIMUM (Kana)
        0x91, 0x2,   //   OUTPUT (Data,Var,Abs)
        0x95, 0x1,   //   REPORT_COUNT (1)
        0x75, 0x3,   //   REPORT_SIZE (3)
        0x91, 0x3,   //   OUTPUT (Cnst,Var,Abs) 91 03
        0x95, 0x6,   //   REPORT_COUNT (6)
        0x75, 0x8,   //   REPORT_SIZE (8)
        0x15, 0x0,   //   LOGICAL_MINIMUM (0)
        0x25, 0x65,  //   LOGICAL_MAXIMUM (101)
        0x5, 0x7,    //   USAGE_PAGE (Keyboard)
        0x19, 0x0,   //   USAGE_MINIMUM (Reserved (no event indicated))
        0x29, 0x65,  //   USAGE_MAXIMUM (Keyboard Application)
        0x81, 0x0,   //   INPUT (Data,Ary,Abs)
        0x0C0        // END_COLLECTION
    };

    usb_error_t error = USB_SUCCESS;

    active_device = usb_FindDevice(NULL, NULL, USB_SKIP_HUBS);

    switch ((unsigned)event) {
        case USB_DEFAULT_SETUP_EVENT: {
            const usb_control_setup_t *setup = event_data;

            
        
            if (!memcmp(hid_report_check, setup, sizeof(*setup))) {
                char msg[18];
                sprintf(msg, "DEVICE: %u", (uint24_t) active_device);
                os_PutStrFull(msg);
                os_NewLine();
                error = usb_ScheduleTransfer(usb_GetDeviceEndpoint(active_device, 0), (void *)hid_report_descriptor, 63, NULL, NULL);
                sprintf(msg, "REP_ERR: %u", error);
                os_PutStrFull(msg);
                os_NewLine();
                error = USB_IGNORE;

            } else if (setup->bmRequestType == 0x21) {
                error = usb_ScheduleTransfer(usb_GetDeviceEndpoint(active_device, 0), NULL, 0, NULL, NULL);
                char msg[18];
                sprintf(msg, "SET_ERR: %u", error);
                os_PutStrFull(msg);
                os_NewLine();
                error = USB_IGNORE;
            }
            
        }
    }

    return error;               
}

static void store_appvars() {
    static const uint8_t *macros[] = {
        macro1,
        macro2,
        macro3,
        macro4,
        macro5
    };
    static const uint8_t sizes[] = {
        sizeof(macro1),
        sizeof(macro2),
        sizeof(macro3),
        sizeof(macro4),
        sizeof(macro5)
    };

    char appvar_name[] = "HIDM0";

    for (uint8_t i = 0; i < (sizeof(macros)/sizeof(macros[0])); i++) {
        appvar_name[4] += 1;

        uint8_t exists = ti_Open(appvar_name, "r");
        if (exists) {
            ti_Close(exists);
            continue;
        }

        uint8_t macro_handle = ti_Open(appvar_name, "w");
        
        ti_Write(macros[i], sizes[i], 1, macro_handle);
        ti_Close(macro_handle);
    }
}

static void delay_macro(uint16_t delay_length) {
    for (uint16_t i = 0; i <= delay_length; i++) {
        usb_HandleEvents();
    }
}

static uint8_t call_macro(uint8_t macro_index) {
    char macro_msg[9] = "MACRO: 0";
    macro_msg[8] += macro_index;
    
    char appvar_name[] = "HIDM1";
    appvar_name[4] += macro_index;
    uint8_t macro_handle = ti_Open(appvar_name, "r+");
    
    if (!macro_handle) {
        return 1;
    }
    
    // Returns at end of function
    usb_error_t error;

    static uint8_t macro_input_data[8] = {
        0, // Modifier key
        0, // Reserved
        0, // First input
        0,
        0,
        0,
        0,
        0
    };

    for (int i = 0; i < 1000; ++i) {
        int key = ti_GetC(macro_handle);

        if (key <= KEY_NONE ) {
            break;  // End of macro sequence
        } else if (key == KEY_ONCE) { 
            // Key is toggled, 
            key = ti_GetC(macro_handle);
            ++i; 
        } else {
            // Press key
            toggle_hid_key((uint8_t) key, (uint8_t *)&macro_input_data);
            error = (usb_error_t) usb_ScheduleInterruptTransfer(usb_GetDeviceEndpoint(active_device, 0x81), &macro_input_data, 8, NULL, NULL);
            delay_macro(MACRO_DELAY);
        }
        
        // Release key
        toggle_hid_key((uint8_t) key, (uint8_t *)&macro_input_data);
        error = (usb_error_t) usb_ScheduleInterruptTransfer(usb_GetDeviceEndpoint(active_device, 0x81), &macro_input_data, 8, NULL, NULL);
        delay_macro(MACRO_DELAY);
    }

    ti_Close(macro_handle);

    return error;
}

// Exit program and display final error code
static int program_exit(uint8_t error) {
    usb_Cleanup();
    char error_msg[10];
    sprintf(error_msg, "err: %d", error);
    // os_PutStrFull(error_msg);
    os_GetKey();
    return error;
}

int main(void) {
    static const usb_string_descriptor_t product_name = {
        .bLength = sizeof(product_name),
        .bDescriptorType = USB_STRING_DESCRIPTOR,
        .bString = L"Hello, World!",
    };

    static const usb_string_descriptor_t *strings[] = { &product_name };
    static const usb_string_descriptor_t langids = {
        .bLength = sizeof(langids),
        .bDescriptorType = USB_STRING_DESCRIPTOR,
        .bString = {
            [0] = DEFAULT_LANGID,
        },
    };
    static struct {
        usb_configuration_descriptor_t configuration;
        struct {
            usb_interface_descriptor_t interface;
            hid_descriptor_t hid;
            usb_endpoint_descriptor_t endpoints[1];
        } interface0;
    } configuration1 = {
        .configuration = {
            .bLength = sizeof(configuration1.configuration),
            .bDescriptorType = USB_CONFIGURATION_DESCRIPTOR,
            .wTotalLength = sizeof(configuration1),
            .bNumInterfaces = 1,
            .bConfigurationValue = 1,
            .iConfiguration = 0,
            // Must use remote wakeup
            // https://stackoverflow.com/questions/44337151/how-do-you-get-a-composite-usb-linux-device-with-hid-to-wake-up-a-suspended-host
            .bmAttributes = 0xa0, //USB_BUS_POWERED | USB_REMOTE_WAKEUP, 
            .bMaxPower = 500 / 2,
        },
        .interface0 = {
            .interface = {
                .bLength = sizeof(configuration1.interface0.interface),
                .bDescriptorType = USB_INTERFACE_DESCRIPTOR,
                .bInterfaceNumber = 0,
                .bAlternateSetting = 0,
                .bNumEndpoints = sizeof(configuration1.interface0.endpoints) /
                                 sizeof(*configuration1.interface0.endpoints),
                .bInterfaceClass = USB_HID_CLASS,
                .bInterfaceSubClass = 1,
                .bInterfaceProtocol = 1,
                .iInterface = 0,
            },
            .hid = {
                .bLength = sizeof(configuration1.interface0.hid),
                .bDescriptorType = 0x21, // HID descriptor type 0x21
                .bcdHID =  0x0110,
                .bCountryCode = 0,
                .bNumDescriptors = 1,
                .bDescriptorType2 = 34,
                .wDescriptorLength = 63, // sizeof(hid_report_descriptor)
            },
            .endpoints = {
                [0] = {
                    .bLength = sizeof(configuration1.interface0.endpoints[0]),
                    .bDescriptorType = USB_ENDPOINT_DESCRIPTOR,
                    .bEndpointAddress = USB_DEVICE_TO_HOST | 1,
                    .bmAttributes = USB_INTERRUPT_TRANSFER,
                    .wMaxPacketSize = 0x0008,
                    .bInterval = 1,
                },
            },
        },
    };
    static const usb_configuration_descriptor_t *configurations[] = {
        &configuration1.configuration,
    };
    static const usb_device_descriptor_t device = {
        .bLength = sizeof(device),
        .bDescriptorType = USB_DEVICE_DESCRIPTOR, //1
        .bcdUSB = 0x0200,
        .bDeviceClass = 0,
        .bDeviceSubClass = 0,
        .bDeviceProtocol = 0,
        .bMaxPacketSize0 = 0x40,
        .idVendor = 0x3f0, // HP, Inc
        .idProduct = 0x24, // KU-0316
        .bcdDevice = 0x0300,
        .iManufacturer = 0,
        .iProduct = 0,
        .iSerialNumber = 0,
        .bNumConfigurations = sizeof(configurations) / sizeof(*configurations),
    };
    static const usb_standard_descriptors_t standard = {
        .device = &device,
        .configurations = configurations,
        .langids = &langids,
        .numStrings = sizeof(strings) / sizeof(*strings),
        .strings = strings,
    };

    // This array will be populated with the HID input data
    static uint8_t input_data[8] = {
        0, // Modifier key
        0, // Reserved
        0, // First input
        0,
        0,
        0,
        0,
        0
    };

    // This data is sent when too many keys are pressed at once
    static uint8_t roll_over_data[8] = {
        0, // Modifier key
        0, // Reserved
        KEY_ERR_OVF, // First input
        KEY_ERR_OVF,
        KEY_ERR_OVF,
        KEY_ERR_OVF,
        KEY_ERR_OVF,
        KEY_ERR_OVF
    };

    store_appvars();
    os_SetCursorPos(1, 0);

    // Copy of kb_Data to compare for changes
    uint8_t last_kb_Data[8] = {1,1,1,1,1,1,1,1};

    usb_error_t error;
    if ((error = usb_Init(handleUsbEvent, NULL, &standard,
                          USB_DEFAULT_INIT_FLAGS)) == USB_SUCCESS) {
        // os_PutStrFull("Success!");
        os_NewLine();

        while(1) {
            // Handle events
            usb_HandleEvents();

            // Update keypadc state (kb_Data)
            kb_Scan();

            uint8_t input_changed = 0;

            // Determine whether input has changed
            for (uint8_t i = 0; i <= 7; i++) {
                if (last_kb_Data[i] != kb_Data[i]) {
                    input_changed = 1;
                    break;
                }
            }

            // Don't do anything if no keys are updated
            if (!input_changed) {
                continue;
            }

            // Clear input array
            for (uint8_t i = 0; i <= 7; i++) {
                input_data[i] = KEY_NONE;
            }
            
            uint8_t rollover_err = 0; // To determine if too many keys are pressed at once

            // Converts keypadc data into GetGSC codes
            // Then adds them to input_data array
            for (uint8_t key = 1, byte = 7; byte; --byte) {
                for (uint8_t mask = 1; mask; mask <<= 1, ++key) {
                    if (kb_Data[byte] & mask) {

                        switch(key) {
                            case sk_Clear: // Exit if clear is pressed
                                return program_exit(error);
                            case sk_Graph: // Call macro 5
                                call_macro(4);
                                break;
                            case sk_Trace: // Call macro 4
                                call_macro(3);
                                break;
                            case sk_Zoom: // Call macro 3
                                call_macro(2);
                                break;
                            case sk_Window: // Call macro 2
                                call_macro(1);
                                break;
                            case sk_Yequ: // Call macro 1
                                call_macro(0);
                                break;
                        }

                        // Toggle key
                        rollover_err = toggle_sk_key(key, &input_data[0]);
                    }
                }
            }

            // Update previous input state
            for (uint8_t i = 0; i <= 7; i++) {
                last_kb_Data[i] = kb_Data[i];
            }

            // Send rollover data instead of normal data to host
            // If more than 6 keys are pressed at once
            if (rollover_err) {
                error = (usb_error_t) usb_ScheduleInterruptTransfer(usb_GetDeviceEndpoint(active_device, 0x81), &roll_over_data, 8, NULL, NULL);
                continue;
            }

            // Send input data to host
            error = (usb_error_t) usb_ScheduleInterruptTransfer(usb_GetDeviceEndpoint(active_device, 0x81), &input_data, 8, NULL, NULL);
        }
    }

    program_exit(error);
}

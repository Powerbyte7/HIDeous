#include <usbdrvce.h>
#include <stdio.h>
#include <stdlib.h>
#include <tice.h>
#include <string.h>

#include "hid.h"

#define DEFAULT_LANGID 0x0409

static usb_error_t set_configuration(usb_device_t device, uint8_t index) {
    usb_error_t error = USB_SUCCESS;
    size_t length = usb_GetConfigurationDescriptorTotalLength(device, index), transferred;
    usb_configuration_descriptor_t *descriptor = malloc(length);
    if (error == USB_SUCCESS && !descriptor)
        error = USB_ERROR_NO_MEMORY;
    if (error == USB_SUCCESS)
        error = usb_GetConfigurationDescriptor(device, index, descriptor, length, &transferred);
    if (error == USB_SUCCESS)
        error = usb_SetConfiguration(device, descriptor, transferred);
    free(descriptor);
    return error;
}

static uint8_t debug_counter = 0;

static usb_device_t active_device;

static usb_error_t handleUsbEvent(usb_event_t event, void *event_data,
                                  usb_callback_data_t *callback_data) {
    static const usb_control_setup_t check_idle_request = {
        .bmRequestType = USB_HOST_TO_DEVICE | USB_VENDOR_REQUEST | USB_RECIPIENT_INTERFACE, // 0x21
        .bRequest = 0x0a, // Set idle
        .wValue = 0,
        .wIndex = 0,
        .wLength = 0,
    };

    static const hid_report_request_t check_hid_report_request = {
        .bmRequestType = 0x81, // USB_DEVICE_TO_HOST | USB_STANDARD_REQUEST | USB_RECIPIENT_INTERFACE, 
        .bDescriptorIndex = 0x0, 
        .bDescriptorType = 0x22, // HID report
        .wDescriptorLength = 63,
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

    static const uint8_t hid_report_check[8] = {
        0x81,
        0x6,
        0x0,
        0x22,
        0x0,
        0x0,
        0x63
    };
    
    usb_error_t error = USB_SUCCESS;

    active_device = usb_FindDevice(NULL, active_device, USB_SKIP_HUBS);

    static const usb_control_setup_t transfer_setup = {
        .bmRequestType = 0,
        .bRequest = 0,
        .wValue = 0,
        .wIndex = 0,
        .wLength = 0,
    };

    switch ((unsigned)event) {
        case USB_DEFAULT_SETUP_EVENT: {
            const usb_control_setup_t *setup = event_data;
            const uint8_t *hid2 = event_data;
            const hid_report_request_t *hid = event_data;
            if (hid2[0] == 0x81) {
                error = USB_IGNORE;
                printf("DEVICE:%02X", active_device);

                error = usb_ScheduleTransfer(usb_GetDeviceEndpoint(active_device, 0), hid_report_descriptor, 63, NULL, NULL);
                printf("ERROR:%d", error);
                //usb_ScheduleDefaultControlTransfer(device, &transfer_setup, hid_report_descriptor, NULL, NULL);
            } else if (hid2[0] == 0x21) {
                error = usb_ScheduleTransfer(usb_GetDeviceEndpoint(active_device, 0), NULL, 0, NULL, NULL);
            }

            // if (!memcmp(hid2, &hid_report_check, sizeof(hid_report_check))) {
            //     printf("HID1");
            //     error = USB_SUCCESS;
            // } else if (!memcmp(hid, &check_hid_report_request, sizeof(check_hid_report_request))) {
            //     printf("HID2");
            //     error = USB_SUCCESS;
            // } else if (!memcmp(hid, &check_hid_report_request, 4)) {
            //     printf("HID3");
            //     error = USB_SUCCESS;
            // }
            printf("%d:%02X ",debug_counter, hid2[0]);
            debug_counter++;

        }
    }     
    return error;               
}


int main(void) { 
    

    static const usb_string_descriptor_t *strings[] = { /* TODO: Add custom strings here */ };
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
                    .bInterval = 255,
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
        .idVendor = 0x0451,
        .idProduct = 0xE008,
        .bcdDevice = 0x0220,
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

    os_SetCursorPos(1, 0);

    static const usb_control_setup_t interrupt_setup = {
        .bmRequestType = 0,
        .bRequest = 0,
        .wValue = 0,
        .wIndex = 0,
        .wLength = 0,
    };

    static const uint8_t input_data[8] = {
        0, // Modifier key
        0, // Reserved
        KEY_A, // First input
        0,
        0,
        0,
        0,
        0
    };



    usb_error_t error;
    if ((error = usb_Init(handleUsbEvent, NULL, &standard,
                          USB_DEFAULT_INIT_FLAGS)) == USB_SUCCESS) {
        printf("Success!\n");

        //usb_ScheduleInterruptTransfer(usb_GetDeviceEndpoint(active_device, 0x02), input_data, 8, NULL, NULL);

        while (!os_GetCSC()) {
            usb_HandleEvents();
        }
    }

    usb_Cleanup();
    printf("error: %d", error);
    os_GetKey();
}
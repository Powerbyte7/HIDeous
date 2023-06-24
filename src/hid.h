#ifdef __cplusplus
extern "C"
{
#endif

typedef struct hid_descriptor {
    uint8_t  bLength;
    uint8_t  bDescriptorType;       // HID descriptor type 0x21
    uint16_t bcdHID;                // Major(8), Minor(4), Revision(4)
    uint8_t  bCountryCode;
    uint8_t  bNumDescriptors;       // Total number of HID report descriptors for the interface.
    uint8_t  bDescriptorType2;   // Type of HID class descriptor
    uint16_t wDescriptorLength;  // Length of HID class descriptor
} hid_descriptor_t;

typedef struct hid_report_request {
    uint8_t bmRequestType;
    uint8_t bDescriptorIndex;
    uint8_t bDescriptorType;
    uint8_t wDescriptorLength;
} hid_report_request_t;

typedef enum keyboard_key {
    KEY_NULL = 0,
    KEY_A = 4,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
} keyboard_key_t;

#ifdef __cplusplus
extern "C"
}
#endif
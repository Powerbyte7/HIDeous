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

#ifdef __cplusplus
extern "C"
}
#endif
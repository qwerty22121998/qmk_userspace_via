#pragma once

#define SPLIT_USB_DETECT
#define SPLIT_WPM_ENABLE
#define DYNAMIC_KEYMAP_LAYER_COUNT 3
#define OLED_TIMER 30000 // OLED timeout in ms

#define LUNA
#define SUGAR

#ifdef SUGAR
#    define SPLIT_TRANSACTION_IDS_USER USER_SYNC_KEY_CNTR
#endif

#ifdef LUNA
#    define DISABLE_LEFT_WPM
#endif

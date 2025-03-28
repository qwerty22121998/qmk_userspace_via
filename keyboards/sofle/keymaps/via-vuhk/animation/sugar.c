#ifdef OLED_ENABLE
#    include "transactions.h"
uint16_t keyCntr = 0;
#    define OLED_SUGAR_HEIGHT 104

#    define OLED_SUGAR_WIDTH 32

bool pixelInvert = false;
void setPixel(char* pixels, uint8_t h, uint8_t w, bool pix) {
    //    oled_write_pixel(w, h, pix);
    uint16_t byteIdx  = w + (h / 8) * OLED_SUGAR_WIDTH;
    int8_t   byteMask = 1 << (h % 8);
    if (pixelInvert) pix = !pix;
    if (pix) {
        pixels[byteIdx] |= byteMask;
    } else {
        pixels[byteIdx] &= ~byteMask;
    }
}

bool getPixel(char* pixels, uint8_t h, uint8_t w) {
    uint16_t byteIdx  = w + (h / 8) * OLED_SUGAR_WIDTH;
    int8_t   byteMask = 1 << (h % 8);
    bool     pix      = (pixels[byteIdx] & byteMask) != 0;
    if (pixelInvert) pix = !pix;
    return pix;
}

uint32_t rand_basic(void) {
    static uint32_t seed = 0;
    seed                 = 1664525 * seed + 1013904223;
    return seed;
}

#    define OLED_SUGAR_WITH_DOWN 1

#    define OLED_SUGAR_PIXELS (OLED_SUGAR_HEIGHT * OLED_SUGAR_WIDTH)
#    define OLED_SUGAR_BYTES (OLED_SUGAR_PIXELS / 8)
typedef signed char lineIdx_t;
lineIdx_t*          activeSugar = NULL;

char* pixels = NULL;

void oled_sugar(void) {
    // return;

    if (activeSugar == NULL) {
        activeSugar = malloc(OLED_SUGAR_HEIGHT);
        if (activeSugar != NULL) {
            memset(activeSugar, -1, OLED_SUGAR_HEIGHT);
        } else {
#    ifdef PRINT_DB
            uprintf("activeSugar %X failed to inialise\n", activeSugar);
#    endif
            return;
        }
    }

    if (pixels == NULL) {
        pixels = malloc(OLED_SUGAR_BYTES);
        if (pixels != NULL) {
            memset(pixels, 0, OLED_SUGAR_BYTES);
        } else {
#    ifdef PRINT_DB
            uprintf("pixels %X failed to inialise\n", pixels);
#    endif
            return;
        }
    }

    static uint8_t sugarCntr = 0;

    for (int16_t h = OLED_SUGAR_HEIGHT - 2; h >= 0; h--) {
        lineIdx_t w = activeSugar[h];

        if (w < 0 || w >= OLED_SUGAR_WIDTH) {
            continue;
        }
        lineIdx_t wn = -1;
        bool      left;
        bool      right;
#    ifdef OLED_SUGAR_WITH_DOWN
        uint32_t       r       = rand_basic();
        const uint32_t thresh1 = UINT32_MAX / 3;
        const uint32_t thresh2 = UINT32_MAX - thresh1;
        left                   = r < thresh1;
        right                  = r > thresh2;
#    else
        left  = ((int32_t)rand_basic()) < 0;
        right = !left;
#    endif
        if (left) {
            if (w > 0) {
                if (!getPixel(pixels, h + 1, w - 1)) {
                    wn = w - 1;
                }
            }
        } else if (right) {
            if (w < OLED_SUGAR_WIDTH - 1) {
                if (!getPixel(pixels, h + 1, w + 1)) {
                    wn = w + 1;
                }
            }
        } else { // centre
            if (!getPixel(pixels, h + 1, w)) {
                wn = w;
            }
        }
        if (wn == -1) {
            if (!getPixel(pixels, h + 1, w)) {
                wn = w;
            }
        }
        if (wn != -1) {
            setPixel(pixels, h + 1, wn, true);
            activeSugar[h + 1] = wn;
            setPixel(pixels, h, w, false);
        }

        activeSugar[h] = -1;
    }
    if (sugarCntr != (keyCntr & 0xFF)) {
        sugarCntr++;
        lineIdx_t w    = OLED_SUGAR_WIDTH / 2;
        bool      left = false;
        if (((int32_t)rand_basic()) < 0) {
            w--;
            left = true;
        }
        bool full = false;
        while (getPixel(pixels, 0, w)) {
            if (left) {
                if (w == 0) {
                    w = OLED_SUGAR_WIDTH - 1;
                } else if (w == OLED_SUGAR_WIDTH / 2) {
                    full = true;
                    break;
                } else {
                    w--;
                }
            } else {
                if (w == OLED_SUGAR_WIDTH - 1) {
                    w = 0;
                } else if (w == OLED_SUGAR_WIDTH / 2 - 1) {
                    full = true;
                    break;
                } else {
                    w++;
                }
            }
        }

        if (!full) {
            //        if(!getPixel(pixels, 0, w)) {
            setPixel(pixels, 0, w, true);
            activeSugar[0] = w;
        } else {
            pixelInvert = !pixelInvert;
            // we where not able to add a pixel (we where full so add one now that we have inverted)
            sugarCntr--;
            oled_sugar();
        }
    }
    rand_basic(); // just here to rotate the seed
    if (is_oled_on()) {
        oled_write_raw(pixels, OLED_SUGAR_BYTES);
    }
}

bool sugar_task_user(void) {
    led_usb_state = host_keyboard_led_state();
    if (is_keyboard_master()) {
        if (timer_elapsed32(oled_timer) > OLED_TIMER) {
            oled_off();
            return false;
        } else {
            oled_on();
        }
        render_status();
    } else {
        oled_sugar();
        oled_set_cursor(0, 14);
        oled_write_P(PSTR("WPM: "), false);
        oled_write(get_u8_str(get_current_wpm(), ' '), false);
    }
    return false;
}

void user_sync_a_update_keyCntr_on_other_board(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {
    keyCntr = *((const uint16_t*)in_data);
}

#endif

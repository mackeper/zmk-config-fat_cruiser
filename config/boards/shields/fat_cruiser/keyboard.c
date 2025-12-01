#include <zmk/display.h>
#include <zmk/event/focus.h>
#include <zmk/event/position.h>
#include <zmk/event/layer.h>
#include <zmk/event/key.h>

#include "custom_status_screen.c"

void zmk_keyboard_init(void) {
    zmk_display_set_custom_status_screen(custom_status_screen_draw);
}

#include <zmk/display.h>
#include <zmk/event/focus.h>
#include <zmk/event/position.h>
#include <zmk/event/layer.h>
#include <zmk/event/key.h>

#include <zmk/display/widgets/layer_name.h>
#include <zmk/display/widgets/battery.h>
#include <zmk/display/widgets/keymap.h>

#include <zmk/display/widgets/test_pattern.h>  // For test drawing

// Custom status screen with a test pattern
void custom_status_screen_draw(zmk_display_t *display) {
    zmk_display_clear(display);

    // Draw a test pattern
    zmk_display_test_pattern(display, 0, 0, 128, 64, ZMK_DISPLAY_TEST_PATTERN_CROSS);

    // Optionally draw text
    zmk_display_draw_string(display, "Test Screen", 0, 0, ZMK_DISPLAY_COLOR_WHITE);
}

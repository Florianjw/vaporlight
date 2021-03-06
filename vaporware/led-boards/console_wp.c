#include "console_wp.h"

#include <stdint.h>

#include "config.h"
#include "console.h"
#include "debug.h"
#include "term.h"

/*
 * Functions for WP_ADJUST operation.
 */

/*
 * State to be held between commands
 */
static int selected_led = 0;

typedef enum {
	L_WHITEPOINT,
	L_BRIGHTNESS
} adjust_selection_t;

static adjust_selection_t selected_line = L_WHITEPOINT;

static uint8_t brightnesses[MODULE_LENGTH] = {[0 ... MODULE_LENGTH - 1] = 0xff};

static const char *ROW_LED_INDEX  = "LED index:  ";
static const char *ROW_WHITEPOINT = "Correction: ";
static const char *ROW_BRIGHTNESS = "Brightness:   ";

static const char *HELP_LINE =
	"Select LED: a/d or h/l; Change line: TAB; Edit: e; Exit: q" CRLF
	"Adjust by 1: w/s or k/j; Adjust by 256: W/S or K/J";

/*
 * Displays the whitepoint and brightness status of the LEDs in the
 * WP_ADJUST input mask.
 */
void show_status_wp() {
	debug_string(TERM_CLEAR);

	debug_string(ROW_LED_INDEX);
	for (int i = 0; i < MODULE_LENGTH; i++) {
		debug_int(i, 4);
		debug_string(" ");
	}

	debug_string(CRLF CRLF);
	
	debug_string(ROW_WHITEPOINT);
	for (int i = 0; i < MODULE_LENGTH; i++) {
		if (selected_led == i && selected_line == L_WHITEPOINT) {
			debug_string(TERM_INVERT);
		}
		
		int phy = config.physical_led[i];
		debug_hex(config.white_correction[phy], 4);

		debug_string(TERM_STANDARD " ");
	}

	debug_string(CRLF CRLF);

	debug_string(ROW_BRIGHTNESS);
	for (int i = 0; i < MODULE_LENGTH; i++) {
		if (selected_led == i && selected_line == L_BRIGHTNESS) {
			debug_string(TERM_INVERT);
		}
		
		debug_hex(brightnesses[i], 2);

		debug_string(TERM_STANDARD "   ");
	}

	debug_string(CRLF CRLF);
	debug_string(HELP_LINE);
}

static void inc16_unless_overflow(uint16_t *data, int16_t delta) {
	if ((delta < 0 && *data < -delta) ||
	    (delta > 0 && *data > 0xffff - delta)) {
		return;
	}

	*data += delta;
}

static void inc8_unless_overflow(uint8_t *data, int8_t delta) {
	if ((delta < 0 && *data < -delta) ||
	    (delta > 0 && *data > 0xff - delta)) {
		return;
	}

	*data += delta;
}

static void inc_selected(int16_t delta) {
	int phy;
	uint8_t delta8;
	
	switch (selected_line) {
	case L_WHITEPOINT:
		phy = config.physical_led[selected_led];
		inc16_unless_overflow(&(config.white_correction[phy]), delta);
		break;
	case L_BRIGHTNESS:
		if (delta < 0x7f && delta > -0x80) {
			delta8 = (uint8_t) delta;
			inc8_unless_overflow(&(brightnesses[selected_led]), delta8);
		}
		break;
	default:
#ifdef SIMULATION
		debug_string("BUG: Unknown value for selected_line");
#else
		error(ER_BUG, STR_WITH_LEN("Unknown value for selected_line"), EA_PANIC);
#endif
		break;
	}
}

static void run_direct_edit() {
	// First, place the cursor at the right value:
	// Reset to top left.
	debug_string(TERM_CURSOR_RESET);
	// Move to correct line
	int down = (selected_line == L_WHITEPOINT) ?
		2 : 4;
	for (int i = 0; i < down; i++) {
		debug_string("\n");
	}
	// Move to correct column
	int right = strlen(ROW_WHITEPOINT) + selected_led * 5;
	for (int i = 0; i < right; i++) {
		debug_string(TERM_CURSOR_RIGHT);
	}

	// Second, erase the value there.
	debug_string("    ");
	if (selected_line == L_WHITEPOINT) {
		debug_string("\b\b\b\b");
	} else {
		debug_string("\b\b");
	}

	// Get the user input
	char input_line[4];
	unsigned int input;
	int pos = 0;

	debug_getline(input_line, 4);
	if (parse_int(input_line, &pos, &input, 16) != E_SUCCESS) {
		// If an error happens, ignore the input.
		return;
	}

	if (selected_line == L_WHITEPOINT) {
		// Because of the length restriction in debug_getline,
		// input cannot possibly overflow.
		int phy = config.physical_led[selected_led];
		config.white_correction[phy] = input;
	} else {
		if (input <= 0xff) {
			brightnesses[selected_led] = input;
		}
	}
}

static void leds_dark() {
	for (int i = 0; i < MODULE_LENGTH; i++) {
		if (led_set_brightness(i, 0) != E_SUCCESS) {
			error(ER_BUG, STR_WITH_LEN("Error while updating LEDs"), EA_PANIC);
		}
	}

	if (led_send_frame()) {
		error(ER_BUG, STR_WITH_LEN("Error while sending frame"), EA_PANIC);
	}
}

/*
 * Runs a command (i.e. keypress) in whitepoint adjust operation.
 *
 * Returns 1 if the console should exit and continue with normal mode.
 */
int run_command_wp() {
	char input = debug_getchar();

	switch(input) {
	case 'q':
		debug_string(TERM_CLEAR);
		console_set_operation(PROMPT);
		leds_dark();
		// Exit early here to avoid resetting the brightnesses to the stored
		// values.
		return 0;
		break;
	case '\t':
		if (selected_line == L_WHITEPOINT) {
			selected_line = L_BRIGHTNESS;
		} else {
			selected_line = L_WHITEPOINT;
		}
		break;
	case 'd':
	case 'l':
		if (selected_led < MODULE_LENGTH - 1) {
			selected_led++;
		}
		break;
	case 'a':
	case 'h':
		if (selected_led > 0) {
			selected_led--;
		}
		break;
	case 'w':
	case 'k':
		inc_selected(1);
		break;
	case 'W':
	case 'K':
		inc_selected(0x100);
		break;
	case 's':
	case 'j':
		inc_selected(-1);
		break;
	case 'S':
	case 'J':
		inc_selected(-0x100);
		break;
	case 'e':
		run_direct_edit();
		break;
	default:
		break;
	}

	// Update all LEDs.
#ifndef SIMULATION
	for (int i = 0; i < MODULE_LENGTH; i++) {
		if (led_set_brightness(config.physical_led[i], brightnesses[i]) != E_SUCCESS) {
			error(ER_BUG, STR_WITH_LEN("Error while updating LEDs"), EA_PANIC);
		}
	}

	if (led_send_frame()) {
		error(ER_BUG, STR_WITH_LEN("Error while sending frame"), EA_PANIC);
	}
#endif
	
	return 0;
}

#include "scr_lucky_num.h"
#include "screens.h"

#include <stdlib.h>

#define LUCKY_NUM_MIN_DIGITS 3
#define LUCKY_NUM_MAX_DIGITS 6

static uint8_t lucky_selected_digits = LUCKY_NUM_MIN_DIGITS;

static void view_scr_lucky_num();

view_dynamic_t dyn_view_item_lucky_num = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_lucky_num
};

// this is the screen struct for lucky number screen, it will be used in screen transition
view_screen_t scr_lucky_num = {
	&dyn_view_item_lucky_num,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

//this would be the function to draw the UI of lucky number screen, it will be called in the view function and also when we need to update the screen after user interaction
static void draw_ui() {
	view_render.clear();
	view_render.setTextSize(1);
	view_render.setTextColor(WHITE);
	view_render.setCursor(2, 4);
	view_render.print("Lucky Number");

	view_render.setCursor(2, 20);
	view_render.print("Digits:");
	view_render.setCursor(60, 20);
	view_render.print(lucky_selected_digits);

	view_render.setCursor(2, 36);
	view_render.print("Use Up/Down to select");
	view_render.setCursor(2, 50);
	view_render.print("Press Mode to start");
	view_render.update();
}

// this function will show the countdown and generate the lucky number, it will be called when user press the mode button
static void show_countdown_and_generate() {
    // countdown from 3 to 1 with 1 second interval, and play sound effect for each number
    view_render.clear();
	view_render.setTextSize(2);
	view_render.setTextColor(WHITE);
	for (int c = 3; c >= 1; c--) {
		view_render.clear();
        view_render.fillScreen(WHITE);
		view_render.setTextSize(9); // bigger size to fill most of the screen
		view_render.setTextColor(BLACK);
		view_render.setCursor(45, 0);
		view_render.print(c);
		view_render.update();
		BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
		sys_ctrl_delay_ms(600);
	}
	// generate random number with selected digits
	uint32_t min = 1;
	for (uint8_t i = 1; i < lucky_selected_digits; i++) min *= 10;
	uint32_t max = min * 10 - 1;
	uint32_t range = (max - min) + 1;
	uint32_t r = min + (rand() % range);
	// show result
	view_render.fillRect(0, 0, 128, 64, WHITE); // fill background with white
	view_render.setTextSize(1);
	view_render.setTextColor(BLACK);
	view_render.setCursor(2, 4);
	view_render.print("Result:");
	view_render.setTextSize(3);
	view_render.setCursor(8, 18);
	view_render.print(r);
	view_render.update();
	BUZZER_PlaySound(BUZZER_SOUND_BANG);
	sys_ctrl_delay_ms(2000);
}

void view_scr_lucky_num() {
	draw_ui();
}

void scr_lucky_num_handle(ak_msg_t* msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		view_render.initialize();
		view_render_display_on();
		draw_ui();
	} break;
	case AC_DISPLAY_BUTTON_UP_RELEASED: {
        APP_DBG_SIG("AC_DISPLAY_BUTTON_UP_RELEASED\n");
		if (lucky_selected_digits < LUCKY_NUM_MAX_DIGITS) {
			lucky_selected_digits++;
			BUZZER_PlaySound(BUZZER_SOUND_CLICK);
		}
		draw_ui();
	} break;
	case AC_DISPLAY_BUTTON_DOWN_RELEASED: {
        APP_DBG_SIG("AC_DISPLAY_BUTTON_DOWN_RELEASED\n");
		if (lucky_selected_digits > LUCKY_NUM_MIN_DIGITS) {
			lucky_selected_digits--;
			BUZZER_PlaySound(BUZZER_SOUND_CLICK);
		}
		draw_ui();
	} break;
    case AC_DISPLAY_BUTTON_UP_LONG_PRESSED: {
        APP_DBG_SIG("AC_DISPLAY_BUTTON_UP_LONG_PRESSED\n");
		SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
		BUZZER_PlaySound(BUZZER_SOUND_CLICK);
    } break;
	case AC_DISPLAY_BUTTON_DOWN_LONG_PRESSED: {
        APP_DBG_SIG("AC_DISPLAY_BUTTON_DOWN_LONG_PRESSED\n");
		SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
		BUZZER_PlaySound(BUZZER_SOUND_CLICK);
    } break;
    case AC_DISPLAY_BUTTON_MODE_RELEASED: {
        APP_DBG_SIG("AC_DISPLAY_BUTTON_MODE_RELEASED\n");
		show_countdown_and_generate();
	} break;
	default:
		break;
	}
}


#include "button.h"

#include "sys_dbg.h"

#include "app.h"
#include "app_bsp.h"
#include "app_dbg.h"
#include "app_if.h"

#include "task_list.h"

#include "scr_archery_game.h"

button_t btn_mode;
button_t btn_up;
button_t btn_down;

void btn_mode_callback(void* b) {
	button_t* me_b = (button_t*)b;
	switch (me_b->state) {
	case BUTTON_SW_STATE_PRESSED: {
		APP_DBG("[%s] BUTTON_SW_STATE_PRESSED\n", __func__);
		// task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_MODE_PRESSED);
	} break;

	case BUTTON_SW_STATE_LONG_PRESSED: {
		APP_DBG("[%s] BUTTON_SW_STATE_LONG_PRESSED\n", __func__);
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_MODE_LONG_PRESSED);
	} break;

	case BUTTON_SW_STATE_RELEASED: {
		APP_DBG("[%s] BUTTON_SW_STATE_RELEASED\n", __func__);
		if (ar_game_state != GAME_OFF) {
			task_post_pure_msg(AR_GAME_ARROW_ID, AR_GAME_ARROW_SHOOT);
		}
		else {
			task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_MODE_RELEASED);

			// Reset timer show idle screen
			timer_set(	AC_TASK_DISPLAY_ID, \
						AC_DISPLAY_SHOW_IDLE, \
						AC_DISPLAY_IDLE_INTERVAL, \
						TIMER_ONE_SHOT);
		}
	} break;

	default:
		break;
	}
}

void btn_up_callback(void* b) {
	button_t* me_b = (button_t*)b;
	switch (me_b->state) {
	case BUTTON_SW_STATE_PRESSED: {
		APP_DBG("[%s] BUTTON_SW_STATE_PRESSED\n", __func__);
		// task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_UP_PRESSED);
	} break;

	case BUTTON_SW_STATE_LONG_PRESSED: {
		APP_DBG("[%s] BUTTON_SW_STATE_LONG_PRESSED\n", __func__);
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_UP_LONG_PRESSED);
	} break;

	case BUTTON_SW_STATE_RELEASED: {
		APP_DBG("[%s] BUTTON_SW_STATE_RELEASED\n", __func__);
		if (ar_game_state != GAME_OFF) {
			task_post_pure_msg(AR_GAME_ARCHERY_ID, AR_GAME_ARCHERY_UP);
		}
		else {
			task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_UP_RELEASED);
			// Reset timer show idle screen
			timer_set(	AC_TASK_DISPLAY_ID, \
						AC_DISPLAY_SHOW_IDLE, \
						AC_DISPLAY_IDLE_INTERVAL, \
						TIMER_ONE_SHOT);
		}
	} break;

	default:
		break;
	}
}

void btn_down_callback(void* b) {
	button_t* me_b = (button_t*)b;
	switch (me_b->state) {
	case BUTTON_SW_STATE_PRESSED: {
		APP_DBG("[%s] BUTTON_SW_STATE_PRESSED\n", __func__);
		// task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_DOWN_PRESSED);
	} break;

	case BUTTON_SW_STATE_LONG_PRESSED: {
		APP_DBG("[%s] BUTTON_SW_STATE_LONG_PRESSED\n", __func__);
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_DOWN_LONG_PRESSED);
	}	
		break;

	case BUTTON_SW_STATE_RELEASED: {
		APP_DBG("[%s] BUTTON_SW_STATE_RELEASED\n", __func__);
		if (ar_game_state != GAME_OFF) {
			task_post_pure_msg(AR_GAME_ARCHERY_ID, AR_GAME_ARCHERY_DOWN);
		}
		else {
			task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTTON_DOWN_RELEASED);
			// Reset timer show idle screen
			timer_set(	AC_TASK_DISPLAY_ID, \
						AC_DISPLAY_SHOW_IDLE, \
						AC_DISPLAY_IDLE_INTERVAL, \
						TIMER_ONE_SHOT);
		}
	} break;

	default:
		break;
	}
}

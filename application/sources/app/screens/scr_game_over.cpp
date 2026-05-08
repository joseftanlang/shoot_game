#include "scr_game_over.h"
#include "app_eeprom.h"

/*****************************************************************************/
/* Variable Declaration - game over */
/*****************************************************************************/
ar_game_score_t gamescore;

/*****************************************************************************/
/* View - game over */
/*****************************************************************************/
static void view_scr_game_over();

view_dynamic_t dyn_view_item_game_over = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_over
};

view_screen_t scr_game_over = {
	&dyn_view_item_game_over,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

void view_scr_game_over() {
	// Screen
	view_render.fillScreen(WHITE);
	view_render.setTextSize(2);
	view_render.setTextColor(BLACK);
	view_render.setCursor(11, 10);
	view_render.print("GAME OVER");
	view_render.setTextSize(1);
	view_render.setTextColor(BLACK);
	view_render.setCursor(16, 35);
	view_render.print("Your score:");
	view_render.setCursor(81, 35);
	view_render.print(gamescore.score_now);
	// Icon
	view_render.drawBitmap(10, 	48,	icon_restart,	15,	15,	0);
	view_render.drawBitmap(55, 	47,	chart_icon,		17,	16,	0);
	view_render.drawBitmap(100,	48,	icon_go_home,	16,	16,	0);
}

/*****************************************************************************/
/* Handle - game over */
/*****************************************************************************/
void rank_ranking() {
    // Check if current score ties with 1st place
    if (gamescore.score_now == gamescore.score_1st) {
    }
    else if (gamescore.score_now > gamescore.score_1st) {
        gamescore.score_3rd = gamescore.score_2nd;
        gamescore.score_2nd = gamescore.score_1st;
        gamescore.score_1st = gamescore.score_now;
    }
    else if (gamescore.score_now == gamescore.score_2nd) {
    }
    else if (gamescore.score_now > gamescore.score_3rd) {
        gamescore.score_3rd = gamescore.score_now;
    }
}
// void rank_ranking() {
// 	if (gamescore.score_now > gamescore.score_1st) {
// 		gamescore.score_3rd = gamescore.score_2nd;
// 		gamescore.score_2nd = gamescore.score_1st;
// 		gamescore.score_1st = gamescore.score_now;
// 	}
// 	else if (gamescore.score_now > gamescore.score_2nd) {
// 		gamescore.score_3rd = gamescore.score_2nd;
// 		gamescore.score_2nd = gamescore.score_now;
// 	}
// 	else if (gamescore.score_now > gamescore.score_3rd) {
// 		gamescore.score_3rd = gamescore.score_now;
// 	}
// }

void scr_game_over_handle(ak_msg_t* msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		APP_DBG_SIG("SCREEN_ENTRY\n");
		// View render
		view_render.initialize();
		view_render_display_on();

		uint32_t score_save = gamescore.score_now;
		// Read score 1st, 2nd, 3rd
		ar_game_score_read(&gamescore);

		gamescore.score_now = score_save;

		// Reorganize
		rank_ranking();
		ar_game_score_write(&gamescore);

		// Timer show idle screen
		timer_set(	AC_TASK_DISPLAY_ID, \
					AC_DISPLAY_SHOW_IDLE, \
					AC_DISPLAY_IDLE_INTERVAL, \
					TIMER_ONE_SHOT);
	} break;

	case AC_DISPLAY_BUTTON_MODE_RELEASED: {
		APP_DBG_SIG("AC_DISPLAY_BUTTON_MODE_RELEASED\n");
		// Save score and go Menu game
		ar_game_score_write(&gamescore);
		ar_game_score = 10;
		SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
		BUZZER_PlaySound(BUZZER_SOUND_CLICK);
	} break;

	case AC_DISPLAY_BUTTON_UP_RELEASED: {
		APP_DBG_SIG("AC_DISPLAY_BUTTON_UP_RELEASED\n");
		// Save score and go Charts
		ar_game_score_write(&gamescore);
		ar_game_score = 10;
		SCREEN_TRAN(scr_charts_game_handle, &scr_charts_game );
		BUZZER_PlaySound(BUZZER_SOUND_CLICK);
	} break;

	case AC_DISPLAY_BUTTON_DOWN_RELEASED: {
		APP_DBG_SIG("AC_DISPLAY_BUTTON_DOWN_RELEASED\n");
		// Save score and restart game
		ar_game_score_write(&gamescore);
		ar_game_score = 10;
		SCREEN_TRAN(scr_archery_game_handle, &scr_archery_game );
		BUZZER_PlaySound(BUZZER_SOUND_CLICK);
	} break;

	case AC_DISPLAY_SHOW_IDLE: {
		APP_DBG_SIG("AC_DISPLAY_SHOW_IDLE\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE);
		scr_idle_set_return_screen(scr_game_over_handle, &scr_game_over);
		SCREEN_TRAN(scr_idle_handle, &scr_idle);
	} break;

	default:
		break;
	}
}

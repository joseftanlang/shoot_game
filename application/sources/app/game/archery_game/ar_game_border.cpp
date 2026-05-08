#include "ar_game_border.h"
#include "ar_game_meteoroid.h"
#include "ar_game_archery.h"

#define AR_GAME_LEVEL_UP_SCORE          (300)

ar_game_border_t border;
uint32_t ar_game_score					 = 10;
static uint32_t ar_game_next_level_score = AR_GAME_LEVEL_UP_SCORE;

void ar_game_border_handle(ak_msg_t *msg) {
	switch (msg->sig) {
	case AR_GAME_BORDER_SETUP: {
		APP_DBG_SIG("AR_GAME_BORDER_SETUP\n");
		border.x			= AXIS_X_BORDER;
		border.visible		= WHITE; //this is the line in the game to show if its detected or not
		border.action_image = AR_GAME_BORDER_ACTION_IMAGE_1;
	} break;

	case AR_GAME_LEVEL_UP: {
		APP_DBG_SIG("AR_GAME_LEVEL_UP\n");
		if (ar_game_score >= ar_game_next_level_score) {
			if (settingsetup.meteoroid_speed < AR_GAME_SETTING_METEOROID_SPEED_MAX) {
				settingsetup.meteoroid_speed++;
			}
			ar_game_next_level_score += AR_GAME_LEVEL_UP_SCORE;
		}
	} break;

	case AR_GAME_CHECK_GAME_OVER: {
		APP_DBG_SIG("AR_GAME_CHECK_GAME_OVER\n");
		for (uint8_t i = 0; i < NUM_METEOROIDS; i++) {
			if (meteoroid[i].visible == WHITE && meteoroid[i].x <= (border.x - 3)) {
				task_post_pure_msg(AR_GAME_SCREEN_ID, AR_GAME_RESET);
				break;
			}
		}
	} break;

	case AR_GAME_BORDER_RESET: {
		APP_DBG_SIG("AR_GAME_BORDER_RESET\n");
		border.visible = BLACK;
	} break;

	default:
		break;
	}
}
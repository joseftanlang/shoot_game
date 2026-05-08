#include "ar_game_arrow.h"
#include "ar_game_archery.h"
#include "scr_archery_game.h"

ar_game_arrow_t arrow[MAX_NUM_ARROW];

void ar_game_arrow_handle(ak_msg_t *msg) {
	switch (msg->sig) {
	case AR_GAME_ARROW_SETUP: {
		APP_DBG_SIG("AR_GAME_ARROW_SETUP\n");
		for (uint8_t i = 0; i < MAX_NUM_ARROW; i++) {
			arrow[i].x			  = 0;
			arrow[i].y			  = 0;
			arrow[i].visible	  = BLACK;
			arrow[i].action_image = AR_GAME_ARROW_ACTION_IMAGE_1;
		}
	} break;

	case AR_GAME_ARROW_RUN: {
		APP_DBG_SIG("AR_GAME_ARROW_RUN\n");
		for (uint8_t i = 0; i < MAX_NUM_ARROW; i++) {
			if (arrow[i].visible == WHITE) {
				arrow[i].x += settingsetup.arrow_speed;
				if (arrow[i].x >= MAX_AXIS_X_ARROW) {
					arrow[i].visible = BLACK;
					arrow[i].x		 = 0;
					if (settingsetup.num_arrow < MAX_NUM_ARROW) {
						settingsetup.num_arrow++;
						archery.action_image = AR_GAME_ARCHERY_ACTION_IMAGE_1;
					}
				}
			}
		}
	} break;

	case AR_GAME_ARROW_SHOOT: {
		APP_DBG_SIG("AR_GAME_ARROW_SHOOT\n");
		if (settingsetup.num_arrow == 0) {
			BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
		}
		else {
			for (uint8_t i = 0; i < MAX_NUM_ARROW; i++) {
				if (arrow[i].visible == BLACK) {
					settingsetup.num_arrow--;
					arrow[i].visible = WHITE;
					arrow[i].y		 = archery.y - 5;
					if (settingsetup.num_arrow < 1) {
						archery.action_image = AR_GAME_ARCHERY_ACTION_IMAGE_2;
					}
					BUZZER_PlaySound(BUZZER_SOUND_CLICK);
					break;
				}
			}
		}
	} break;

	case AR_GAME_ARROW_RESET: {
		APP_DBG_SIG("AR_GAME_ARROW_RESET\n");
		for (uint8_t i = 0; i < MAX_NUM_ARROW; i++) {
			arrow[i].visible = BLACK;
		}
	} break;

	default:
		break;
	}
}

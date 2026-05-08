#include "ar_game_bang.h"

#include "ar_game_meteoroid.h"
#include "ar_game_arrow.h"

ar_game_bang_t bang[NUM_BANG];

void ar_game_bang_handle(ak_msg_t *msg) {
	switch (msg->sig) {
	case AR_GAME_BANG_SETUP: {
		APP_DBG_SIG("AR_GAME_BANG_SETUP\n");
		for (uint8_t i = 0; i < NUM_BANG; i++) {
            // Clear bang and reset action image
			bang[i].visible		 = BLACK;
			bang[i].action_image = AR_GAME_BANG_ACTION_IMAGE_1;
		}
	} break;

	case AR_GAME_BANG_UPDATE: {
		APP_DBG_SIG("AR_GAME_BANG_UPDATE\n");
		for (uint8_t i = 0; i < NUM_BANG; i++) {
			if (bang[i].visible == WHITE) {
                // Update action image
				bang[i].action_image++;
			}
			if (bang[i].action_image >= AR_GAME_BANG_ACTION_IMAGE_END) {
                // Clear bang and show meteoroid
				bang[i].action_image = AR_GAME_BANG_ACTION_IMAGE_1;
				bang[i].visible		 = BLACK;
				meteoroid[i].visible = WHITE;
			}
		}
	} break;

	case AR_GAME_BANG_RESET: {
		APP_DBG_SIG("AR_GAME_BANG_RESET\n");
		for (uint8_t i = 0; i < NUM_BANG; i++) {
            // Clear bang
			bang[i].visible = BLACK;
		}
	} break;

	default:
		break;
	}
}
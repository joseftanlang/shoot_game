#include "ar_game_meteoroid.h"

#include "ar_game_arrow.h"
#include "ar_game_bang.h"
#include "ar_game_border.h"
#include "scr_archery_game.h"

#define IS_COLLIDE(x1,y1,w1,h1,x2,y2,w2,h2) (((x1) < ((x2) + (w2))) && \
                                             (((x1) + (w1)) > (x2)) && \
                                             ((y1) < ((y2) + (h2))) && \
                                             (((y1) + (h1)) > (y2)))

#define RANDOM_METEOROID_X() ((rand() % 39) + 130)
#define RANDOM_METEOROID_ACTION_IMAGE() (rand() % AR_GAME_METEOROID_ACTION_IMAGE_3 + AR_GAME_METEOROID_ACTION_IMAGE_1)

ar_game_meteoroid_t meteoroid[NUM_METEOROIDS];

void ar_game_meteoroid_handle(ak_msg_t *msg) {
	switch (msg->sig) {
	case AR_GAME_METEOROID_SETUP: {
		APP_DBG_SIG("AR_GAME_METEOROID_SETUP\n");
		for (uint8_t i = 0; i < NUM_METEOROIDS; i++) {
			meteoroid[i].y			  = AXIS_Y_METEOROID_START + (i * AXIS_Y_METEOROID_STEP);
			meteoroid[i].x			  = RANDOM_METEOROID_X();
			meteoroid[i].visible	  = WHITE;
			meteoroid[i].action_image = RANDOM_METEOROID_ACTION_IMAGE();
		}
	} break;

	case AR_GAME_METEOROID_RUN: {
		APP_DBG_SIG("AR_GAME_METEOROID_RUN\n");
		for (uint8_t i = 0; i < NUM_METEOROIDS; i++) {
			if (meteoroid[i].visible == WHITE) {
				meteoroid[i].x -= settingsetup.meteoroid_speed;
				//undo this if u want to make the bitmap change of the 3 contsantly
				// meteoroid[i].action_image++;
				// if (meteoroid[i].action_image > AR_GAME_METEOROID_ACTION_IMAGE_3) {
				// 	meteoroid[i].action_image = AR_GAME_METEOROID_ACTION_IMAGE_1;
				// }
			}
		}
	} break;

	case AR_GAME_METEOROID_DETONATOR: {
		APP_DBG_SIG("AR_GAME_METEOROID_DETONATOR\n");
		for (uint8_t i = 0; i < NUM_METEOROIDS; i++) {
			if (meteoroid[i].visible == WHITE) {
				for (uint8_t j = 0; j < MAX_NUM_ARROW; j++) {
					if (arrow[j].visible == WHITE) {
                       if (IS_COLLIDE(arrow[j].x, arrow[j].y, SIZE_BITMAP_ARROW_X, SIZE_BITMAP_ARROW_Y, \
                                      meteoroid[i].x, meteoroid[i].y, SIZE_BITMAP_METEOROIDS_X, SIZE_BITMAP_METEOROIDS_Y))
                        {
                            // Clear arrow and meteoroid
                            meteoroid[i].visible	  = BLACK;
                            arrow[j].visible		  = BLACK;

                            // Show bang
                            bang[i].visible			  = WHITE;
                            bang[i].action_image	  = AR_GAME_BANG_ACTION_IMAGE_1;
                            bang[i].x				  = (meteoroid[i].x > 5 ? meteoroid[i].x - 5 : 0);
                            bang[i].y				  = meteoroid[i].y + 2;

                            // Reset arrow and meteoroid
                            arrow[j].x				  = 0;
                            arrow[j].y				  = 0;
                            meteoroid[i].x			  = RANDOM_METEOROID_X();
                            meteoroid[i].action_image = RANDOM_METEOROID_ACTION_IMAGE();
                            if (settingsetup.num_arrow < MAX_NUM_ARROW) {
                                settingsetup.num_arrow++;
                            }

                            // Update score and play sound
                            ar_game_score += 10;
                            BUZZER_PlaySound(BUZZER_SOUND_BANG);
                            break;
                        }
					}
				}
			}
		}
	} break;

	case AR_GAME_METEOROID_RESET: {
		APP_DBG_SIG("AR_GAME_METEOROID_RESET\n");
		for (uint8_t i = 0; i < NUM_METEOROIDS; i++) {
			meteoroid[i].visible = BLACK;
		}
	} break;

	default:
		break;
	}
}

#ifndef __AR_GAME_METEOROID_H__
#define __AR_GAME_METEOROID_H__

#include <stdio.h>

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "task_display.h"

#include "buzzer.h"
#include "scr_archery_game.h"

#define NUM_METEOROIDS              (5)
#define SIZE_BITMAP_METEOROIDS_X    (20)
#define SIZE_BITMAP_METEOROIDS_Y    (10)

#define AXIS_Y_METEOROID_START   (2)
#define AXIS_Y_METEOROID_STEP    (10)
enum {
    AR_GAME_METEOROID_ACTION_IMAGE_1 = 1,
    AR_GAME_METEOROID_ACTION_IMAGE_2,
    AR_GAME_METEOROID_ACTION_IMAGE_3,
    AR_GAME_METEOROID_ACTION_IMAGE_END
};

typedef struct {
    bool visible;
    uint32_t x, y;
    uint8_t action_image;
} ar_game_meteoroid_t;

extern ar_game_meteoroid_t meteoroid[NUM_METEOROIDS];

#endif //__AR_GAME_METEOROID_H__
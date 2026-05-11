#include "scr_dino_run.h"
#include <stdlib.h>

#include "screens.h"
#include "app_eeprom.h"
#include "screens_bitmap.h"

#define DINO_RUN_TICK_SIG              (AR_GAME_DEFINE_SIG + 60)
#define OBSTACLE_SPAWN_INTERVAL_MS     (1500)
#define DINO_JUMP_HEIGHT               (20)
#define DINO_RUN_TICK_INTERVAL_MS      (110)
#define DINO_RUN_OBSTACLE_WIDTH        (15)
#define DINO_RUN_OBSTACLE_HEIGHT       (20)
#define DINO_RUN_DINO_WIDTH            (20)
#define DINO_RUN_DINO_HEIGHT           (20)
#define DINO_RUN_GROUND_Y              (LCD_HEIGHT - DINO_RUN_DINO_HEIGHT)
#define DINO_RUN_JUMP_DURATION_MS      (500)
#define DINO_RUN_MAX_OBSTACLES        (3)
#define DINO_RUN_OBSTACLE_SPAWN_SIG    (AR_GAME_DEFINE_SIG + 61)
#define DINO_START_SCORE               (0)
#define DINO_SCORE_INCREMENT           (10)
#define DINO_RUN_OFF                   (0)

typedef struct {
    int16_t x;
    int16_t y;
    uint8_t type; // which cactus sprite
    uint8_t width;
    uint8_t height;
    uint8_t active;
} obstacle_t;

uint8_t dino_run_state;
dino_run_setting_t dino_run_settingsetup;
static int16_t dino_run_score = 0;
static obstacle_t obstacles[DINO_RUN_MAX_OBSTACLES];
static int16_t dino_y = DINO_RUN_GROUND_Y;
static uint8_t is_jumping = 0;
static uint32_t jump_elapsed_ms = 0;
static int16_t obstacle_speed = 2;


static void dino_run_spawn_obstacle() {
    for (uint8_t i = 0; i < DINO_RUN_MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) { // If slot free
            obstacles[i].x = LCD_WIDTH; // Spawn at the right edge
            obstacles[i].y = DINO_RUN_GROUND_Y; // On the ground
            obstacles[i].type = rand() % 3; // choose cactus sprite
            obstacles[i].width = DINO_RUN_OBSTACLE_WIDTH;
            obstacles[i].height = DINO_RUN_OBSTACLE_HEIGHT;
            obstacles[i].active = 1;
            break;
        }
    }
}

static void dino_run_reset() {
    dino_run_score = DINO_START_SCORE;
    // Load speed from game settings (range 1-5)
    ar_game_setting_read(&settingdata);
    dino_run_settingsetup.speed = settingdata.meteoroid_speed;
    obstacle_speed = (int16_t)dino_run_settingsetup.speed + 1; // ensure minimum movement
    dino_y = DINO_RUN_GROUND_Y;
    is_jumping = 0;
    jump_elapsed_ms = 0;
    for (uint8_t i = 0; i < DINO_RUN_MAX_OBSTACLES; i++) {
        obstacles[i].x = -DINO_RUN_OBSTACLE_WIDTH; // Start off-screen
        obstacles[i].y = DINO_RUN_GROUND_Y;
        obstacles[i].active = 0;
        obstacles[i].type = 0;
        obstacles[i].width = DINO_RUN_OBSTACLE_WIDTH;
        obstacles[i].height = DINO_RUN_OBSTACLE_HEIGHT;
    }
}

/* removed local dino_end_game; use global scr_game_over screen instead */

static void dino_run_update() {
    // Update obstacle positions
    for (uint8_t i = 0; i < DINO_RUN_MAX_OBSTACLES; i++) {
        if (obstacles[i].active) { // If the obstacle is on-screen
            obstacles[i].x -= obstacle_speed; // Move left
            if (obstacles[i].x + obstacles[i].width < 0) {
                obstacles[i].active = 0;
                obstacles[i].x = -obstacles[i].width;
            }
        }
    }

    // Handle jumping
    if (is_jumping) {
        jump_elapsed_ms += DINO_RUN_TICK_INTERVAL_MS;
        if (jump_elapsed_ms < DINO_RUN_JUMP_DURATION_MS) {
            dino_y = DINO_RUN_GROUND_Y - (DINO_JUMP_HEIGHT * jump_elapsed_ms / DINO_RUN_JUMP_DURATION_MS);
        } else {
            dino_y = DINO_RUN_GROUND_Y;
            is_jumping = 0;
            jump_elapsed_ms = 0;
        }
    }

    // Check for collisions (rectangle overlap)
    int16_t dino_x = 10;
    for (uint8_t i = 0; i < DINO_RUN_MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;
        int16_t ox = obstacles[i].x;
        int16_t oy = obstacles[i].y;
        int16_t ow = obstacles[i].width;
        int16_t oh = obstacles[i].height;
        bool overlap = !(dino_x + DINO_RUN_DINO_WIDTH <= ox ||
                         dino_x >= ox + ow ||
                         dino_y + DINO_RUN_DINO_HEIGHT <= oy ||
                         dino_y >= oy + oh);
        if (overlap) {
            // Collision detected -> game over
            dino_run_state = DINO_RUN_OFF;
            timer_remove_attr(AC_TASK_DISPLAY_ID, DINO_RUN_TICK_SIG);
            timer_remove_attr(AC_TASK_DISPLAY_ID, DINO_RUN_OBSTACLE_SPAWN_SIG);
                // set global gamescore so game-over screen shows score
                gamescore.score_now = dino_run_score;
            BUZZER_PlaySound(BUZZER_SOUND_LOWSCORE);
            SCREEN_TRAN(scr_game_over_handle, &scr_game_over);
            return;
        }
    }

    // Increment score
    dino_run_score += DINO_SCORE_INCREMENT;
}

static void dino_run_draw() {
    view_render.clear();
    // Draw dino
    view_render.drawBitmap(10, dino_y, dino_icon, DINO_RUN_DINO_WIDTH, DINO_RUN_DINO_HEIGHT, WHITE);
    // Draw obstacles
    //rand() % 3 == 0 ? cactus_icon_1 : rand() % 3 == 1 ? cactus_icon_2 : cactus_icon_3, obstacles[i].x, obstacles[i].y)
    for (uint8_t i = 0; i < DINO_RUN_MAX_OBSTACLES; i++) {
        if (obstacles[i].active) {
            const unsigned char* bmp = (obstacles[i].type == 0) ? cactus_icon_1 : (obstacles[i].type == 1) ? cactus_icon_2 : cactus_icon_3;
            view_render.drawBitmap(obstacles[i].x, obstacles[i].y, bmp, obstacles[i].width, obstacles[i].height, WHITE);
        }
    }
    // Draw score
    view_render.setTextSize(1);
    view_render.setTextColor(WHITE);
    view_render.setCursor(80, 10);
    view_render.print("Score: ");
    view_render.print(dino_run_score);
    view_render.update();
}

static void view_scr_dino_run();

view_dynamic_t dyn_view_item_dino_run = {
    {
        .item_type = ITEM_TYPE_DYNAMIC,
    },
    view_scr_dino_run
};

view_screen_t scr_dino_run = {
    &dyn_view_item_dino_run,
    ITEM_NULL,
    ITEM_NULL,

    .focus_item = 0,
};

static void view_scr_dino_run() {
    dino_run_draw();
}

void scr_dino_run_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case SCREEN_ENTRY: {
        view_render.initialize();
        view_render_display_on();
        dino_run_reset();
        dino_run_state = 1; // start immediately when entering
        dino_run_draw(); // draw initial frame
        timer_set(AC_TASK_DISPLAY_ID, DINO_RUN_TICK_SIG, DINO_RUN_TICK_INTERVAL_MS, TIMER_PERIODIC);
        timer_set(AC_TASK_DISPLAY_ID, DINO_RUN_OBSTACLE_SPAWN_SIG, OBSTACLE_SPAWN_INTERVAL_MS, TIMER_PERIODIC);
    } break;

    case DINO_RUN_TICK_SIG: {
        if (dino_run_state == DINO_RUN_OFF) {
            return;
        }
        dino_run_update();
        dino_run_draw();
    } break;

    case DINO_RUN_OBSTACLE_SPAWN_SIG: {
        if (dino_run_state == DINO_RUN_OFF) {
            return;
        }
        dino_run_spawn_obstacle();
    } break;

    case AC_DISPLAY_BUTTON_DOWN_RELEASED: {`
        if (dino_run_state == DINO_RUN_OFF) {
            dino_run_state = 1; // Start the game
            dino_run_reset();
        }
        BUZZER_PlaySound(BUZZER_SOUND_CLICK);
    } break;

    case AC_DISPLAY_BUTTON_DOWN_LONG_PRESSED: {
        SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
        BUZZER_PlaySound(BUZZER_SOUND_CLICK);
    }

    case AC_DISPLAY_BUTTON_UP_RELEASED: {
        // Start jump if not already jumping
        if (dino_run_state == 1 && !is_jumping) {
            is_jumping = 1;
            jump_elapsed_ms = 0;
        }
        BUZZER_PlaySound(BUZZER_SOUND_CLICK);
    } break;

    case AC_DISPLAY_BUTTON_MODE_RELEASED: {
        timer_remove_attr(AC_TASK_DISPLAY_ID, DINO_RUN_TICK_SIG);
        timer_remove_attr(AC_TASK_DISPLAY_ID, DINO_RUN_OBSTACLE_SPAWN_SIG);
        SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
        BUZZER_PlaySound(BUZZER_SOUND_CLICK);
    } break;

    default:
        break;
    }
}


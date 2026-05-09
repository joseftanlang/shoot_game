#include "scr_fruit_game.h"

#include <stdlib.h>

#include "screens.h"

#define FRUIT_GAME_NUM_FRUITS            (6)
#define FRUIT_GAME_NUM_GOOD              (3)
#define FRUIT_GAME_NUM_BAD               (3)

#define FRUIT_GAME_TICK_SIG              (AR_GAME_DEFINE_SIG + 50)
#define FRUIT_GAME_TICK_INTERVAL_MS      (110)

#define FRUIT_GAME_TOP_UI_H              (12)
#define FRUIT_GAME_FRUIT_W               (8)
#define FRUIT_GAME_FRUIT_H               (8)
#define FRUIT_GAME_BASKET_W              (16)
#define FRUIT_GAME_BASKET_H              (8)
#define FRUIT_GAME_BASKET_Y              (54)
#define FRUIT_GAME_BASKET_STEP           (6)

typedef struct {
    int16_t x;
    int16_t y;
    uint8_t type;
} fruit_object_t;

uint8_t fruit_game_state;
fruit_game_setting_t fruit_settingsetup;

static int16_t fruit_game_score = 0;
static uint8_t good_collected_mask = 0;
static uint8_t bad_collected_mask = 0;
static int16_t basket_x = 0;
static fruit_object_t falling_fruit;

static const uint8_t* const fruit_bitmap_table[FRUIT_GAME_NUM_FRUITS] = {
    bitmap_fruit_good_1,
    bitmap_fruit_good_2,
    bitmap_fruit_good_3,
    bitmap_fruit_bad_1,
    bitmap_fruit_bad_2,
    bitmap_fruit_bad_3,
};

static void fruit_game_spawn_fruit() {
    falling_fruit.type = rand() % FRUIT_GAME_NUM_FRUITS;
    falling_fruit.x = rand() % (LCD_WIDTH - FRUIT_GAME_FRUIT_W);
    falling_fruit.y = FRUIT_GAME_TOP_UI_H;
}

static void fruit_game_reset() {
    fruit_game_score = 0;
    good_collected_mask = 0;
    bad_collected_mask = 0;
    fruit_settingsetup.speed = 2;
    basket_x = (LCD_WIDTH - FRUIT_GAME_BASKET_W) / 2;
    fruit_game_spawn_fruit();
    fruit_game_state = FRUIT_GAME_PLAY;
}

static uint8_t fruit_game_is_overlap_x(int16_t x1, int16_t w1, int16_t x2, int16_t w2) {
    if (x1 + w1 < x2) return 0;
    if (x2 + w2 < x1) return 0;
    return 1;
}

static void fruit_game_process_catch(uint8_t fruit_type) {
    if (fruit_type < FRUIT_GAME_NUM_GOOD) {
        good_collected_mask |= (1U << fruit_type);
        fruit_game_score += 10;
        BUZZER_PlaySound(BUZZER_SOUND_CLICK);
    }
    else {
        bad_collected_mask |= (1U << (fruit_type - FRUIT_GAME_NUM_GOOD));
        fruit_game_score -= 5;
        BUZZER_PlaySound(BUZZER_SOUND_LOWSCORE);
    }

    if (good_collected_mask == 0x07) {
        fruit_game_state = FRUIT_GAME_WIN;
        timer_remove_attr(AC_TASK_DISPLAY_ID, FRUIT_GAME_TICK_SIG);
    }

    if (bad_collected_mask == 0x07) {
        fruit_game_state = FRUIT_GAME_OVER;
        timer_remove_attr(AC_TASK_DISPLAY_ID, FRUIT_GAME_TICK_SIG);
    }
}

static void fruit_game_update() {
    if (fruit_game_state != FRUIT_GAME_PLAY) {
        return;
    }

    falling_fruit.y += fruit_settingsetup.speed;

    if ((falling_fruit.y + FRUIT_GAME_FRUIT_H) >= FRUIT_GAME_BASKET_Y) {
        if (fruit_game_is_overlap_x(falling_fruit.x, FRUIT_GAME_FRUIT_W, basket_x, FRUIT_GAME_BASKET_W)) {
            fruit_game_process_catch(falling_fruit.type);
            fruit_game_spawn_fruit();
            return;
        }
    }

    if (falling_fruit.y > (LCD_HEIGHT - 1)) {
        fruit_game_spawn_fruit();
    }
}

static void fruit_game_draw_mask_line(uint8_t y, const char* label, uint8_t mask) {
    view_render.setCursor(2, y);
    view_render.print(label);
    for (uint8_t i = 0; i < 3; i++) {
        view_render.print((mask & (1U << i)) ? '1' : '0');
    }
}

static void view_scr_fruit_game();

view_dynamic_t dyn_view_item_fruit_game = {
    {
        .item_type = ITEM_TYPE_DYNAMIC,
    },
    view_scr_fruit_game
};

view_screen_t scr_fruit_game = {
    &dyn_view_item_fruit_game,
    ITEM_NULL,
    ITEM_NULL,

    .focus_item = 0,
};

static void view_scr_fruit_game() {
    view_render.clear();

    view_render.setTextSize(1);
    view_render.setTextColor(WHITE);
    view_render.setCursor(2, 2);
    view_render.print("Fruit:");
    view_render.print(fruit_game_score);

    view_render.setCursor(72, 2);
    view_render.print("U< D>");

    view_render.drawLine(0, FRUIT_GAME_TOP_UI_H - 1, LCD_WIDTH, FRUIT_GAME_TOP_UI_H - 1, WHITE);

    fruit_game_draw_mask_line(14, "G:", good_collected_mask);
    fruit_game_draw_mask_line(24, "B:", bad_collected_mask);

    view_render.drawBitmap(
        falling_fruit.x,
        falling_fruit.y,
        fruit_bitmap_table[falling_fruit.type],
        FRUIT_GAME_FRUIT_W,
        FRUIT_GAME_FRUIT_H,
        WHITE
    );

    view_render.drawBitmap(
        basket_x,
        FRUIT_GAME_BASKET_Y,
        bitmap_fruit_basket,
        FRUIT_GAME_BASKET_W,
        FRUIT_GAME_BASKET_H,
        WHITE
    );

    view_render.drawRect(0, 0, LCD_WIDTH, LCD_HEIGHT, WHITE);

    if (fruit_game_state == FRUIT_GAME_OVER || fruit_game_state == FRUIT_GAME_WIN) {
        view_render.fillRect(18, 36, 92, 20, BLACK);
        view_render.drawRect(18, 36, 92, 20, WHITE);
        view_render.setCursor(26, 42);
        if (fruit_game_state == FRUIT_GAME_WIN) {
            view_render.print("YOU WIN +10!");
        }
        else {
            view_render.print("3 BAD TYPES!");
        }
    }
}

void scr_fruit_game_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case SCREEN_ENTRY: {
        APP_DBG_SIG("SCREEN_ENTRY\n");
        view_render.initialize();
        view_render_display_on();
        fruit_game_reset();
        timer_set(
            AC_TASK_DISPLAY_ID,
            FRUIT_GAME_TICK_SIG,
            FRUIT_GAME_TICK_INTERVAL_MS,
            TIMER_PERIODIC
        );
    } break;

    case FRUIT_GAME_TICK_SIG: {
        fruit_game_update();
    } break;

    case AC_DISPLAY_BUTTON_UP_RELEASED: {
        if (fruit_game_state == FRUIT_GAME_PLAY) {
            basket_x -= FRUIT_GAME_BASKET_STEP;
            if (basket_x < 0) {
                basket_x = 0;
            }
            BUZZER_PlaySound(BUZZER_SOUND_CLICK);
        }
    } break;

    case AC_DISPLAY_BUTTON_DOWN_RELEASED: {
        if (fruit_game_state == FRUIT_GAME_PLAY) {
            basket_x += FRUIT_GAME_BASKET_STEP;
            if (basket_x > (LCD_WIDTH - FRUIT_GAME_BASKET_W)) {
                basket_x = LCD_WIDTH - FRUIT_GAME_BASKET_W;
            }
            BUZZER_PlaySound(BUZZER_SOUND_CLICK);
        }
    } break;

    case AC_DISPLAY_BUTTON_MODE_RELEASED: {
        if (fruit_game_state == FRUIT_GAME_OVER || fruit_game_state == FRUIT_GAME_WIN) {
            fruit_game_reset();
            timer_set(
                AC_TASK_DISPLAY_ID,
                FRUIT_GAME_TICK_SIG,
                FRUIT_GAME_TICK_INTERVAL_MS,
                TIMER_PERIODIC
            );
        }
    } break;

    case AC_DISPLAY_BUTTON_UP_LONG_PRESSED: {
        timer_remove_attr(AC_TASK_DISPLAY_ID, FRUIT_GAME_TICK_SIG);
        SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
    } break;

    default:
        break;
    }

}



#include "scr_fruit_game.h"

#include <stdlib.h>

#include "screens.h"
#include "app_eeprom.h"

#define FRUIT_GAME_NUM_FRUITS            (6)
#define FRUIT_GAME_NUM_GOOD              (3)
#define FRUIT_GAME_NUM_BAD               (3)

#define FRUIT_GAME_TICK_SIG              (AR_GAME_DEFINE_SIG + 50)
#define FRUIT_GAME_TICK_INTERVAL_MS      (110)
#define FRUIT_GAME_COUNTDOWN_SIG         (AR_GAME_DEFINE_SIG + 51)
#define FRUIT_GAME_COUNTDOWN_INTERVAL_MS (1000)

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
static uint16_t fruit_game_countdown_seconds = 30;

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
    falling_fruit.y = FRUIT_GAME_TOP_UI_H; // Start just below the top UI
}

static void fruit_game_reset() {
    fruit_game_score = 0;
    good_collected_mask = 0;
    bad_collected_mask = 0;
    // Load speed from game settings (range 1-5)
    ar_game_setting_read(&settingdata);
    fruit_settingsetup.speed = settingdata.meteoroid_speed;
    basket_x = (LCD_WIDTH - FRUIT_GAME_BASKET_W) / 2;
    fruit_game_countdown_seconds = 30;
    fruit_game_spawn_fruit();
    fruit_game_state = FRUIT_GAME_PLAY;
}
// Check if two rectangles overlap on the X axis
static uint8_t fruit_game_is_overlap_x(int16_t x1, int16_t w1, int16_t x2, int16_t w2) {
    if (x1 + w1 < x2) return 0; //
    if (x2 + w2 < x1) return 0;
    return 1;
}
// Process catch and update score and masks
static void fruit_game_process_catch(uint8_t fruit_type) {
    if (fruit_type < FRUIT_GAME_NUM_GOOD) {
        good_collected_mask |= (1U << fruit_type);
        fruit_game_score += 10;
        BUZZER_PlaySound(BUZZER_SOUND_CLICK);
    }
    else {
        bad_collected_mask |= (1U << (fruit_type - FRUIT_GAME_NUM_GOOD)); //1u << 0 for first bad fruit, 1u << 1 for second, etc.
        fruit_game_score -= 5;
        BUZZER_PlaySound(BUZZER_SOUND_LOWSCORE);
    }
    //0x07 means all 3 good fruits collected, 0x07 = 0b00000111
    if (good_collected_mask == 0x07) {
        fruit_game_state = FRUIT_GAME_WIN;
        timer_remove_attr(AC_TASK_DISPLAY_ID, FRUIT_GAME_TICK_SIG);
        timer_remove_attr(AC_TASK_DISPLAY_ID, FRUIT_GAME_COUNTDOWN_SIG);
    }

    if (bad_collected_mask >= 3) {
        fruit_game_state = FRUIT_GAME_OVER;
        timer_remove_attr(AC_TASK_DISPLAY_ID, FRUIT_GAME_TICK_SIG);
        timer_remove_attr(AC_TASK_DISPLAY_ID, FRUIT_GAME_COUNTDOWN_SIG);
    }
}
// Update fruit position and check for catch or miss
static void fruit_game_update() {
    if (fruit_game_state != FRUIT_GAME_PLAY) {
        return;
    }

    falling_fruit.y += fruit_settingsetup.speed; // Move fruit down
    // Check if fruit reaches the basket level
    if ((falling_fruit.y + FRUIT_GAME_FRUIT_H) >= FRUIT_GAME_BASKET_Y) {
        if (fruit_game_is_overlap_x(falling_fruit.x, FRUIT_GAME_FRUIT_W, basket_x, FRUIT_GAME_BASKET_W)) {
            fruit_game_process_catch(falling_fruit.type); // Process catch and update score
            fruit_game_spawn_fruit();
            return;
        }
    }

    if (falling_fruit.y > (LCD_HEIGHT - 1)) {
        fruit_game_spawn_fruit();
    }
}
// For debug: draw mask line
static void fruit_game_draw_mask_line(uint8_t y, const char* label, uint8_t mask) {
    view_render.setCursor(2, y);
    view_render.print(label);
    for (uint8_t i = 0; i < 3; i++) {
        view_render.print((mask & (1U << i)) ? '1' : '0'); // Print '1' if bit is set, '0' if not for each of the 3 fruits in the category
    }
}

static void view_scr_fruit_game();

/// Define the dynamic view item and screen for the fruit game
view_dynamic_t dyn_view_item_fruit_game = {
    {
        .item_type = ITEM_TYPE_DYNAMIC, //this is a dynamic view item that will call the view function to render each frame
    },
    view_scr_fruit_game
};
// Define the screen object for the fruit game
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

    // Display countdown timer on top right
    view_render.setCursor(100, 2);
    view_render.print("T:");
    if (fruit_game_countdown_seconds < 10) {
        view_render.print("0");
    }
    view_render.print(fruit_game_countdown_seconds);

    view_render.drawLine(0, FRUIT_GAME_TOP_UI_H - 1, LCD_WIDTH, FRUIT_GAME_TOP_UI_H - 1, WHITE);

    // fruit_game_draw_mask_line(14, "G:", good_collected_mask); // for debug
    // fruit_game_draw_mask_line(24, "B:", bad_collected_mask);

    view_render.drawBitmap(
        falling_fruit.x,
        falling_fruit.y,
        fruit_bitmap_table[falling_fruit.type], // Get bitmap based on fruit type
        FRUIT_GAME_FRUIT_W,
        FRUIT_GAME_FRUIT_H,
        WHITE
    );
    // Draw the basket
    view_render.drawBitmap(
        basket_x,
        FRUIT_GAME_BASKET_Y,
        bitmap_fruit_basket,
        FRUIT_GAME_BASKET_W,
        FRUIT_GAME_BASKET_H,
        WHITE
    );

    view_render.drawRect(0, 0, LCD_WIDTH, LCD_HEIGHT, WHITE); // Border for the game area

    if (fruit_game_state == FRUIT_GAME_OVER || fruit_game_state == FRUIT_GAME_WIN) {
        view_render.fillRect(12, 28, 104, 32, BLACK); // Clear area for text
        view_render.drawRect(12, 28, 104, 32, WHITE); // Border for text
        view_render.setCursor(22, 34); // Center text in the box
        if (fruit_game_state == FRUIT_GAME_WIN) {
            view_render.print("You Win!");
            view_render.setCursor(20, 44);
            view_render.print("Score: ");
            view_render.print(fruit_game_score);
        }
        else {
            view_render.print("Game Over!");
            view_render.setCursor(20, 44);
            if (fruit_game_countdown_seconds == 0) {
                view_render.print("Time Out!");
            } else {
                view_render.print("3 Bad Fruits!");
            }
            view_render.setCursor(20, 54);
            // view_render.print("Score: ");
            // view_render.print(fruit_game_score);
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
        timer_set(
            AC_TASK_DISPLAY_ID,
            FRUIT_GAME_COUNTDOWN_SIG,
            FRUIT_GAME_COUNTDOWN_INTERVAL_MS,
            TIMER_PERIODIC
        );
    } break;
    // case SCREEN_EXIT: {
    //     timer_remove_attr(AC_TASK_DISPLAY_ID, FRUIT_GAME_TICK_SIG);
    //     timer_remove_attr(AC_TASK_DISPLAY_ID, FRUIT_GAME_COUNTDOWN_SIG);
    // } break;
    case FRUIT_GAME_TICK_SIG: {
        fruit_game_update();
    } break;

    case FRUIT_GAME_COUNTDOWN_SIG: {
        if (fruit_game_state == FRUIT_GAME_PLAY && fruit_game_countdown_seconds > 0) {
            fruit_game_countdown_seconds--;
            if (fruit_game_countdown_seconds == 0) {
                fruit_game_state = FRUIT_GAME_OVER;
                timer_remove_attr(AC_TASK_DISPLAY_ID, FRUIT_GAME_TICK_SIG);
                BUZZER_PlaySound(BUZZER_SOUND_LOWSCORE);
            }
        }
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
            timer_set(
                AC_TASK_DISPLAY_ID,
                FRUIT_GAME_COUNTDOWN_SIG,
                FRUIT_GAME_COUNTDOWN_INTERVAL_MS,
                TIMER_PERIODIC
            );
        }
    } break;

    case AC_DISPLAY_BUTTON_UP_LONG_PRESSED: {
        timer_remove_attr(AC_TASK_DISPLAY_ID, FRUIT_GAME_TICK_SIG);
        timer_remove_attr(AC_TASK_DISPLAY_ID, FRUIT_GAME_COUNTDOWN_SIG);
        SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
    } break;

    case AC_DISPLAY_BUTTON_DOWN_LONG_PRESSED: {
        SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
    }

    // case AC_DISPLAY_SHOW_IDLE: {
    //     timer_remove_attr(AC_TASK_DISPLAY_ID, FRUIT_GAME_TICK_SIG);
    //     timer_remove_attr(AC_TASK_DISPLAY_ID, FRUIT_GAME_COUNTDOWN_SIG);
    //     scr_idle_set_return_screen(scr_fruit_game_handle, &scr_fruit_game);
    //     SCREEN_TRAN(scr_idle_handle, &scr_idle);
    // } break;

    default:
        break;
    }

}



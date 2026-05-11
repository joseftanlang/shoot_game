#ifndef __SCR_DINO_RUN_H__
#define __SCR_DINO_RUN_H__

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "sys_ctrl.h"
#include "sys_dbg.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "task_display.h"
#include "view_render.h"

#include "buzzer.h"

#include "screens.h"
#include "screens_bitmap.h"

//the codes goes here
typedef struct {
	uint8_t speed;
} dino_run_setting_t;

extern dino_run_setting_t dino_run_settingsetup;

#endif //__SCR_DINO_RUN_H__
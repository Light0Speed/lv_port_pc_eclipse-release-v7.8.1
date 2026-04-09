#ifndef HUGE_LIST_DEMO_H
#define HUGE_LIST_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

void huge_list_demo_create(lv_obj_t * parent);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* HUGE_LIST_DEMO_H */

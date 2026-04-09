#ifndef HUGE_LIST_H
#define HUGE_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

typedef struct huge_list_data_source_s {
    int (*get_count)(void);
    int (*get_text)(int index, char *buf, int buf_size);
    const void * (*get_icon)(int index);
    void (*on_click)(int index);
} huge_list_data_source_t;

typedef struct huge_list_page_s huge_list_page_t;

void huge_list_demo_create(lv_obj_t * parent);

huge_list_page_t * huge_list_page_create(lv_obj_t * parent, const huge_list_data_source_t * ds,
                                         int item_height);
void huge_list_page_destroy(huge_list_page_t * hl);
lv_obj_t * huge_list_page_get_obj(huge_list_page_t * hl);
void huge_list_page_refresh(huge_list_page_t * hl);
void huge_list_page_scroll_to(huge_list_page_t * hl, int index, lv_anim_enable_t anim);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* HUGE_LIST_H */

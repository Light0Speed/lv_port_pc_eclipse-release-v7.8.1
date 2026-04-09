#include "huge_list.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HUGE_LIST_PAGE_BATCH_SIZE      8
#define HUGE_LIST_PAGE_BUFFER_COUNT    3
#define HUGE_LIST_PAGE_TEXT_BUF_SIZE   256

struct huge_list_page_s {
    lv_obj_t *page;
    lv_obj_t *scrollable;
    const huge_list_data_source_t *ds;
    lv_task_t *render_task;
    lv_task_t *watch_task;
    int item_height;
    int total_count;
    int current_start;
    int current_end;
    int pending_start;
    int pending_end;
    int next_render_index;
    int last_scroll_y;
    int last_page_height;
    uint8_t rendering;
};

static void huge_list_page_refresh_visible_range(huge_list_page_t *list);

static int huge_list_get_scroll_y(const huge_list_page_t *list)
{
    if(list == NULL || list->scrollable == NULL) return 0;
    return -lv_obj_get_y(list->scrollable);
}

static void huge_list_page_item_event_cb(lv_obj_t *obj, lv_event_t event)
{
    uintptr_t encoded;
    int index;
    huge_list_page_t *list;

    if(event != LV_EVENT_CLICKED) return;
    if(obj == NULL) return;

    encoded = (uintptr_t)lv_obj_get_user_data(obj);
    if(encoded == 0U) return;

    index = (int)(encoded - 1U);
    list = (huge_list_page_t *)lv_obj_get_user_data(lv_obj_get_parent(obj));
    if(list == NULL || list->ds == NULL) return;
    if(list->ds->on_click == NULL) return;

    list->ds->on_click(index);
}

static void huge_list_page_clear_items(huge_list_page_t *list)
{
    lv_obj_t *child;

    if(list == NULL || list->scrollable == NULL) return;

    child = lv_obj_get_child(list->scrollable, NULL);
    while(child != NULL) {
        lv_obj_t *next = lv_obj_get_child(list->scrollable, child);
        lv_obj_del(child);
        child = next;
    }
}

static lv_obj_t *huge_list_page_create_item(huge_list_page_t *list, int index)
{
    lv_obj_t *btn;
    lv_obj_t *label;
    lv_obj_t *sub_label;
    char text_buf[HUGE_LIST_PAGE_TEXT_BUF_SIZE];
    int len;
    char *newline;
    int text_x = 12;

    btn = lv_btn_create(list->scrollable, NULL);
    if(btn == NULL) return NULL;

    lv_obj_set_size(btn, lv_obj_get_width(list->page), list->item_height);
    lv_obj_set_pos(btn, 0, index * list->item_height);
    lv_obj_set_event_cb(btn, huge_list_page_item_event_cb);

    lv_obj_set_user_data(btn, (void *)(uintptr_t)(index + 1));

    if(list->ds->get_icon != NULL) {
        const void *icon_src = list->ds->get_icon(index);
        if(icon_src != NULL) {
            lv_obj_t *icon = lv_label_create(btn, NULL);
            lv_label_set_text(icon, icon_src);
            lv_obj_align(icon, NULL, LV_ALIGN_IN_LEFT_MID, 12, 0);
            text_x = 36;
        }
    }

    len = list->ds->get_text(index, text_buf, sizeof(text_buf) - 1);
    if(len < 0) len = 0;
    if(len >= (int)sizeof(text_buf)) len = (int)sizeof(text_buf) - 1;
    text_buf[len] = '\0';

    newline = strchr(text_buf, '\n');
    if(newline != NULL) {
        *newline = '\0';

        label = lv_label_create(btn, NULL);
        lv_label_set_text(label, text_buf);
        lv_obj_set_pos(label, text_x, 8);

        sub_label = lv_label_create(btn, NULL);
        lv_label_set_text(sub_label, newline + 1);
        lv_obj_set_pos(sub_label, text_x, 30);
        lv_obj_set_style_local_text_color(sub_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    } else {
        label = lv_label_create(btn, NULL);
        lv_label_set_text(label, text_buf);
        lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, text_x, 0);
    }

    return btn;
}

static void huge_list_page_render_task_cb(lv_task_t *task)
{
    huge_list_page_t *list;
    int rendered = 0;

    if(task == NULL) return;

    list = (huge_list_page_t *)task->user_data;
    if(list == NULL) return;
    if(list->page == NULL || list->scrollable == NULL) return;

    while(list->next_render_index <= list->pending_end &&
          rendered < HUGE_LIST_PAGE_BATCH_SIZE) {
        huge_list_page_create_item(list, list->next_render_index);
        list->next_render_index++;
        rendered++;
    }

    if(list->next_render_index > list->pending_end) {
        list->current_start = list->pending_start;
        list->current_end = list->pending_end;
        list->rendering = 0;
        lv_task_set_prio(list->render_task, LV_TASK_PRIO_OFF);
    }
}

static void huge_list_page_watch_task_cb(lv_task_t *task)
{
    huge_list_page_t *list;
    int scroll_y;
    int page_height;

    if(task == NULL) return;

    list = (huge_list_page_t *)task->user_data;
    if(list == NULL || list->page == NULL) return;

    scroll_y = huge_list_get_scroll_y(list);
    page_height = lv_obj_get_height(list->page);

    if(scroll_y != list->last_scroll_y || page_height != list->last_page_height) {
        list->last_scroll_y = scroll_y;
        list->last_page_height = page_height;
        huge_list_page_refresh_visible_range(list);
    }
}

static void huge_list_page_queue_render(huge_list_page_t *list, int start_idx, int end_idx)
{
    if(list == NULL) return;

    if(list->total_count <= 0) {
        huge_list_page_clear_items(list);
        list->current_start = 0;
        list->current_end = -1;
        list->pending_start = 0;
        list->pending_end = -1;
        list->next_render_index = 0;
        list->rendering = 0;
        if(list->render_task != NULL) lv_task_set_prio(list->render_task, LV_TASK_PRIO_OFF);
        return;
    }

    if(start_idx < 0) start_idx = 0;
    if(end_idx >= list->total_count) end_idx = list->total_count - 1;
    if(end_idx < start_idx) end_idx = start_idx;

    if(list->rendering == 0 &&
       list->current_start == start_idx &&
       list->current_end == end_idx) {
        return;
    }

    if(list->rendering != 0 &&
       list->pending_start == start_idx &&
       list->pending_end == end_idx) {
        return;
    }

    huge_list_page_clear_items(list);

    list->pending_start = start_idx;
    list->pending_end = end_idx;
    list->next_render_index = start_idx;
    list->rendering = 1;

    if(list->render_task != NULL) {
        lv_task_set_prio(list->render_task, LV_TASK_PRIO_LOW);
        lv_task_ready(list->render_task);
        huge_list_page_render_task_cb(list->render_task);
    }
}

static void huge_list_page_refresh_visible_range(huge_list_page_t *list)
{
    int scroll_y;
    int page_height;
    int start_idx;
    int end_idx;

    if(list == NULL || list->page == NULL || list->item_height <= 0) return;

    page_height = lv_obj_get_height(list->page);
    if(page_height <= 0) return;

    scroll_y = huge_list_get_scroll_y(list);
    if(scroll_y < 0) scroll_y = 0;

    start_idx = scroll_y / list->item_height;
    end_idx = (scroll_y + page_height - 1) / list->item_height;

    start_idx -= HUGE_LIST_PAGE_BUFFER_COUNT;
    end_idx += HUGE_LIST_PAGE_BUFFER_COUNT;

    if(start_idx < 0) start_idx = 0;
    if(end_idx >= list->total_count) end_idx = list->total_count - 1;

    huge_list_page_queue_render(list, start_idx, end_idx);
}

huge_list_page_t *huge_list_page_create(lv_obj_t *parent,
                                        const huge_list_data_source_t *ds,
                                        int item_height)
{
    huge_list_page_t *list;

    if(parent == NULL || ds == NULL || ds->get_count == NULL || ds->get_text == NULL) return NULL;

    list = (huge_list_page_t *)calloc(1, sizeof(*list));
    if(list == NULL) return NULL;

    list->ds = ds;
    list->item_height = item_height > 0 ? item_height : 56;
    list->total_count = ds->get_count();
    list->current_start = 0;
    list->current_end = -1;
    list->pending_start = 0;
    list->pending_end = -1;

    list->page = lv_page_create(parent, NULL);
    if(list->page == NULL) {
        free(list);
        return NULL;
    }

    lv_obj_set_size(list->page, lv_obj_get_width(parent), lv_obj_get_height(parent));
    lv_obj_set_user_data(list->page, list);
    lv_page_set_scrollbar_mode(list->page, LV_SCROLLBAR_MODE_AUTO);
    lv_page_set_scrl_layout(list->page, LV_LAYOUT_OFF);
    lv_page_set_scrollable_fit2(list->page, LV_FIT_NONE, LV_FIT_NONE);

    list->scrollable = lv_page_get_scrollable(list->page);
    lv_obj_set_width(list->scrollable, lv_obj_get_width(list->page));
    lv_obj_set_height(list->scrollable, list->total_count * list->item_height);

    list->render_task = lv_task_create(huge_list_page_render_task_cb, 20, LV_TASK_PRIO_OFF, list);
    if(list->render_task == NULL) {
        lv_obj_del(list->page);
        free(list);
        return NULL;
    }

    list->watch_task = lv_task_create(huge_list_page_watch_task_cb, 30, LV_TASK_PRIO_MID, list);
    if(list->watch_task == NULL) {
        lv_task_del(list->render_task);
        lv_obj_del(list->page);
        free(list);
        return NULL;
    }

    list->last_scroll_y = -1;
    list->last_page_height = -1;

    huge_list_page_refresh(list);
    return list;
}

void huge_list_page_destroy(huge_list_page_t *list)
{
    if(list == NULL) return;

    if(list->render_task != NULL) {
        lv_task_del(list->render_task);
        list->render_task = NULL;
    }

    if(list->watch_task != NULL) {
        lv_task_del(list->watch_task);
        list->watch_task = NULL;
    }

    if(list->page != NULL) {
        lv_obj_del(list->page);
        list->page = NULL;
    }

    free(list);
}

lv_obj_t *huge_list_page_get_obj(huge_list_page_t *list)
{
    if(list == NULL) return NULL;
    return list->page;
}

void huge_list_page_refresh(huge_list_page_t *list)
{
    if(list == NULL || list->page == NULL) return;

    list->total_count = list->ds->get_count();
    lv_obj_set_width(list->scrollable, lv_obj_get_width(list->page));
    lv_obj_set_height(list->scrollable, list->total_count * list->item_height);

    list->current_start = 0;
    list->current_end = -1;
    huge_list_page_refresh_visible_range(list);
}

void huge_list_page_scroll_to(huge_list_page_t *list, int index, lv_anim_enable_t anim)
{
    lv_obj_t *scrl;
    lv_coord_t target_y;
    lv_coord_t current_y;
    lv_coord_t dist;

    if(list == NULL) return;
    if(index < 0 || index >= list->total_count) return;

    scrl = lv_page_get_scrollable(list->page);
    target_y = -(index * list->item_height);
    current_y = lv_obj_get_y(scrl);
    dist = target_y - current_y;

    if(anim == LV_ANIM_ON) {
        lv_page_scroll_ver(list->page, dist);
    } else {
        lv_obj_set_y(scrl, target_y);
        huge_list_page_refresh_visible_range(list);
    }
}

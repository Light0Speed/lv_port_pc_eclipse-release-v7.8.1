#include "huge_list.h"

#include <stdbool.h>
#include <stdio.h>

#define HUGE_LIST_DEMO_COUNT 2000

typedef struct {
    const char * title;
    const char * subtitle;
} huge_list_demo_item_t;

static huge_list_demo_item_t g_demo_items[HUGE_LIST_DEMO_COUNT];
static bool g_demo_data_ready;

static void huge_list_demo_init_data(void)
{
    static char titles[HUGE_LIST_DEMO_COUNT][32];
    static char subtitles[HUGE_LIST_DEMO_COUNT][32];

    if(g_demo_data_ready) {
        return;
    }

    for(int i = 0; i < HUGE_LIST_DEMO_COUNT; i++) {
        snprintf(titles[i], sizeof(titles[i]), "Song %04d", i + 1);
        snprintf(subtitles[i], sizeof(subtitles[i]), "Artist %c / Album %02d",
                 'A' + (i % 26), (i % 19) + 1);
        g_demo_items[i].title = titles[i];
        g_demo_items[i].subtitle = subtitles[i];
    }

    g_demo_data_ready = true;
}

static int huge_list_demo_get_count(void)
{
    return HUGE_LIST_DEMO_COUNT;
}

static int huge_list_demo_get_text(int index, char *buf, int buf_size)
{
    if(index < 0 || index >= HUGE_LIST_DEMO_COUNT || buf == NULL || buf_size <= 0) {
        return 0;
    }

    return snprintf(buf, (size_t)buf_size, "%s\n%s",
                    g_demo_items[index].title,
                    g_demo_items[index].subtitle);
}

static const void * huge_list_demo_get_icon(int index)
{
    switch(index % 3) {
        case 0: return LV_SYMBOL_AUDIO;
        case 1: return LV_SYMBOL_VIDEO;
        default: return LV_SYMBOL_FILE;
    }
}

static void huge_list_demo_on_click(int index)
{
    if(index < 0 || index >= HUGE_LIST_DEMO_COUNT) {
        return;
    }

    printf("huge_list clicked: %u -> %s / %s\n",
           (unsigned int)index,
           g_demo_items[index].title,
           g_demo_items[index].subtitle);
}

void huge_list_demo_create(lv_obj_t * parent);

void huge_list_demo_create(lv_obj_t * parent)
{
    static huge_list_page_t * huge_list;
    static const huge_list_data_source_t ds = {
        .get_count = huge_list_demo_get_count,
        .get_text = huge_list_demo_get_text,
        .get_icon = huge_list_demo_get_icon,
        .on_click = huge_list_demo_on_click,
    };

    huge_list_demo_init_data();

    if(parent == NULL) {
        parent = lv_scr_act();
    }

    lv_obj_clean(parent);

    huge_list = huge_list_page_create(parent, &ds, 56);
    if(huge_list == NULL) {
        return;
    }

    lv_obj_t * page = huge_list_page_get_obj(huge_list);
    lv_obj_set_size(page, LV_HOR_RES_MAX - 24, LV_VER_RES_MAX - 24);
    lv_obj_align(page, NULL, LV_ALIGN_CENTER, 0, 0);
    huge_list_page_scroll_to(huge_list, 0, LV_ANIM_OFF);
}

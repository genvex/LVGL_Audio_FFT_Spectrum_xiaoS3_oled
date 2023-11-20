#ifndef _MAIN_H_
#define _MAIN_H_

#include <LovyanGFX.hpp>
#include "lvgl.h"
#include "XiaoS3_ssd1306.hpp"
static SSD1306 oled; 
static void lvgl_begin(void);
static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area,
                          lv_color_t *color_p);
lv_disp_t *oled_disp; /*Descriptor of a display driver*/

static void lvgl_begin(void)
{
#define LVGL_HOR_RES (128)
#define LVGL_VER_RES (64)
    // #define DISP_BUF_SIZE (LVGL_HOR_RES * 100)
    oled.init();
    // oled.setRotation(2);
    lv_init();

    static lv_disp_draw_buf_t draw_buf;
    size_t DISP_BUF_SIZE = sizeof(lv_color_t) * (LVGL_HOR_RES * 20); // best operation.
    static lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(
        DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    static lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(
        DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, DISP_BUF_SIZE); /*Initialize the display buffer*/

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);   /*Basic initialization*/

    /*Set up the functions to access to your display*/
    /*Set the resolution of the display*/
    disp_drv.hor_res = LVGL_HOR_RES;
    disp_drv.ver_res = LVGL_VER_RES;
    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = my_disp_flush;
    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf;
    /*Finally register the driver*/
    oled_disp = lv_disp_drv_register(&disp_drv);
}

static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area,
                          lv_color_t *color_p)
{
    int w = (area->x2 - area->x1 + 1);
    int h = (area->y2 - area->y1 + 1);
    /* Start new TFT transaction */
    oled.startWrite();
    /* set the working window */
    oled.setAddrWindow(area->x1, area->y1, w, h);
    oled.writePixelsDMA((lgfx::rgb565_t *)&color_p->full, w * h);
    /* terminate TFT transaction */
    oled.endWrite();
    /* tell lvgl that flushing is done */
    lv_disp_flush_ready(disp);
}


#endif
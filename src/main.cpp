#include <Arduino.h>
#include <driver/i2s.h>
#include "I2SSampler.h"
#include "Processor.h"
#include "main.h"

#define LV_TICK_PERIOD_MS 10
#define LV_HOR_RES_MAX    128
#define LV_VER_RES_MAX    64

#define CENTER_Y      (LV_VER_RES_MAX / 2)
#define WINDOW_HEIGHT (LV_VER_RES_MAX - 8) 

// I2S
#define WINDOW_SIZE  512
#define SAMPLE_SIZE  32
#define I2S_BCK_PIN  -1
#define I2S_WS_PIN   42
#define I2S_DATA_PIN 41
// #define I2S_WS_PIN   44
// #define I2S_DATA_PIN 43
// static void i2s_read_task(void *param);
static void audio_processing_task(void *param);
static void bar_value_update(float *mag);
static void spectrum_draw_event_cb(lv_event_t *e);


static int x_step      = int((LV_HOR_RES_MAX) / SAMPLE_SIZE);
lv_obj_t *spectrum_obj = NULL;

I2SSampler i2s_sampler;
Processor audio_processor(WINDOW_SIZE);
TaskHandle_t processing_task_handle;
float bar_chart_peaks[WINDOW_SIZE] = {0};
float bar_chart[WINDOW_SIZE]       = {0};


// i2s config for reading mic
i2s_config_t i2s_config = {
    .mode        = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate = 64000,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format       = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 2,
    .dma_buf_len          = 1024,
};

// i2s pins
i2s_pin_config_t i2s_pins = {.bck_io_num   = I2S_PIN_NO_CHANGE,
                             .ws_io_num    = I2S_WS_PIN,
                             .data_out_num = I2S_PIN_NO_CHANGE,
                             .data_in_num  = I2S_DATA_PIN};


/* Creates a semaphore to handle concurrent call to lvgl stuff
 * If you wish to call *any* lvgl function from other threads/tasks
 * you should lock on the very same semaphore! */
SemaphoreHandle_t xGuiSemaphore;

void setup() {
    /* Initialize the Serial */
    Serial.begin(115200);
    /* Initialize the LVGL */
    lvgl_begin();
    xGuiSemaphore = xSemaphoreCreateMutex();
    xSemaphoreGive(xGuiSemaphore);
    spectrum_obj = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(spectrum_obj);
    // lv_obj_refresh_ext_draw_size(spectrum_obj);
    lv_obj_set_size(spectrum_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    // lv_obj_set_pos(spectrum_obj, 1, 1);
    lv_obj_center(spectrum_obj);
    lv_obj_clear_flag(spectrum_obj,
                      LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(spectrum_obj,lv_color_black(),0);

    lv_obj_set_style_bg_opa(spectrum_obj,LV_OPA_100,0);
    lv_obj_add_event_cb(spectrum_obj, spectrum_draw_event_cb, LV_EVENT_DRAW_POST,
                        NULL);

    xTaskCreatePinnedToCore(audio_processing_task, "audio processing task",
                            4096, NULL, 2, &processing_task_handle, 0);
    i2s_sampler.start(
        I2S_NUM_0, i2s_pins, i2s_config, WINDOW_SIZE,
        processing_task_handle);  // Put the handle inside the sampler
}

void loop() {
    /* Delay 1 tick (assumes FreeRTOS tick is 10ms) */
    vTaskDelay(pdMS_TO_TICKS(10));
    /* Try to take the semaphore, call lvgl related function on success */
    if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
        lv_task_handler();
        xSemaphoreGive(xGuiSemaphore);
    }
}

static void audio_processing_task(void *param) {
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
    while (true) {
        // wait for some samples to process
        uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
        if (ulNotificationValue > 0) {
            int16_t *samples = i2s_sampler.getCapturedAudioBuffer();
            audio_processor.update(samples);
            bar_value_update(audio_processor.m_energy);
            lv_obj_invalidate(spectrum_obj);
        }
    }
}

static void bar_value_update(float *mag) {
    for (int i = 2, k = 0; k < SAMPLE_SIZE; i += 2, k++) {
        float ave = 0;
        for (int j = 0; j < 2; j++) {
            ave += mag[i + j];
        }
        ave /= 12;  // 4 is for average ,12 for down scale.
        int bar_value      = std::min(float(WINDOW_HEIGHT), ave);
        bar_chart[k]       = (bar_value > bar_chart[k])
                                 ? bar_value
                                 : 0.7 * bar_chart[k] + 0.3 * bar_value;
        bar_chart_peaks[k] = (bar_value > bar_chart_peaks[k])
                                 ? bar_value
                                 : 0.9 * bar_chart_peaks[k] + 0.1 * bar_value;
    }
}

static void spectrum_draw_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_DRAW_POST) {
        lv_obj_t *obj           = lv_event_get_target(e);
        lv_draw_ctx_t *draw_ctx = lv_event_get_draw_ctx(e);
        lv_draw_rect_dsc_t draw_rect_dsc;
        lv_draw_rect_dsc_init(&draw_rect_dsc);
        draw_rect_dsc.bg_opa   = LV_OPA_100;
        draw_rect_dsc.bg_color = lv_color_white();
        lv_draw_line_dsc_t draw_line_dsc;
        lv_draw_line_dsc_init(&draw_line_dsc);
        draw_line_dsc.width = 2;
        draw_line_dsc.color = lv_color_white();

        for (int i = 0; i < SAMPLE_SIZE; i++) {
            lv_area_t _rect;
            _rect.x1 = i * x_step;
            _rect.x2 = i * x_step + 3;
            _rect.y1 = CENTER_Y - int(bar_chart[i] / 2);
            _rect.y2 = CENTER_Y + int(bar_chart[i] / 2);
            lv_draw_rect(draw_ctx, &draw_rect_dsc, &_rect);            
    
            lv_point_t above_line[2];
            /* upside line always 2 px above the bar */
            above_line[0].x = i * x_step;
            above_line[0].y = CENTER_Y - int(bar_chart_peaks[i] / 2) - 2;
            above_line[1].x = i * x_step + 3;
            above_line[1].y = CENTER_Y - int(bar_chart_peaks[i] / 2) - 2;
            lv_draw_line(draw_ctx, &draw_line_dsc, &above_line[0],
                         &above_line[1]);

            lv_point_t blow_line[2];
            /* under line always 2 px below the bar */
            blow_line[0].x = i * x_step;
            blow_line[0].y = CENTER_Y + int(bar_chart_peaks[i] / 2) + 2;      
            blow_line[1].x = i * x_step + 3;
            blow_line[1].y = CENTER_Y + int(bar_chart_peaks[i] / 2) + 2;
            lv_draw_line(draw_ctx, &draw_line_dsc, &blow_line[0],
                         &blow_line[1]);
        }
    }
}

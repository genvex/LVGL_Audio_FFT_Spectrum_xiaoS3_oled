# LVGL_Audio_FFT_Spectrum_xiaoS3_oled


The project aims to achieve real-time visualization of FFT audio data using the OLED display on the Xiao expansion board, driven by the LVGL library. Key components include data visualization on the OLED screen using the LVGL library, high-speed acquisition and preprocessing of audio data on the xiaoS3_senses, as well as synchronous FFT transformation calculations. The project utilizes the I2S microphone component on the xiaoS3_sense development board and the OLED on the Xiao expansion board to achieve the visualization of audio data. It provides an in-depth understanding of the technical details of audio data processing and visualization, and offers reference and application value for real-time monitoring and presentation of audio and music data.


The Xiao-expansion-board extends multiple Grove interfaces and is equipped with an entry-level ssd1306 OLED screen. Traditionally, driving the ssd1306 would require the use of power-hungry ancient artifacts like u8g2. However, with the emergence of many high-performance embedded development boards, the speed of graphic driver updates has accelerated, offering more choices for driving the ssd1306. In this project, we use the LovyanGFX graphic driver library to drive the ssd1306.


### Driving OLED Screen with LovyanGFX

The LovyanGFX library, inspired by TFT_eSPI, has been deeply reformed and is beloved by many hobbyists. It can not only drive common LCD screens but also support some OLED screens. Using this library allows even the simplest entry-level ssd1306 OLED screens to share the drawing functions commonly found in LCD screens. In this project, adaptation for the ssd1306 on the xiao series expansion board has been completed. Furthermore, following the drawing mechanism of the popular lvgl graphics library, the project has been ported, greatly enriching the application scenarios of the ssd1306 on the xiao expansion board.


Use [LovyanGFX](https://github.com/lovyan03/LovyanGFX) as(or like) backend driver for [lvgl](https://github.com/lvgl/lvgl), it is very simple to use, you only need to port one method.

```c
  SSD1306(void)
  {
    { //进行总线控制的设置。
      auto cfg = _bus_instance.config();  // 获取总线设置的结构。
     // I2C设置
      cfg.i2c_port    = 0;          
      cfg.freq_write  = 400000;     
      cfg.freq_read   = 400000; 
      cfg.pin_sda     = SDA;         
      cfg.pin_scl     = SCL;         
      cfg.i2c_addr    = 0x3C; 
      _bus_instance.config(cfg);       
      _panel_instance.setBus(&_bus_instance);      // 把总线设置在面板上。
    }
  }
```

### Porting LVGL Based on SSD1306

Traditional graphic libraries such as the classic TFT_eSPI, Adafruit_GFX, and Lovyan_GFX have been instrumental in enabling users to quickly realize their solutions. However, the emergence of LVGL (Light and Versatile Graphics Library) has provided a more elegant solution for the presentation of embedded developments in final products. LVGL is better suited for embedded systems compared to traditional graphic libraries, offering advantages in performance, resource utilization, and flexibility.

The notion of running LVGL on the ssd1306 was once a lofty dream, but today, we can easily use it in the Arduino programming environment and partake in the wider open-source community, marking a significant advancement.

After the porting process, projects running LVGL on the ssd1303 display seamlessly. When designing programs, it is advisable to specify colors as white or black for clearer display effects.

We hope that you will be inspired by this project and that it will bring convenience to your future work.


```C
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

```

### I2S Microphone Configuration

The xiaoS3_sense board is equipped with both a camera and an I2S microphone, enabling the development of imaginative voice-related projects. The present project leverages the excellent features of the I2S microphone, playing a pivotal role in its implementation. Within the i2s_config, the key parameters to focus on include sample_rate, dma_buf_count, and dma_buf_len. Variations in these parameters will result in noticeable differences in the final data presentation.

A faster sampling rate allows for capturing finer details of sound; however, it's not infinitely scalable. Here, a sampling rate of 64000 is set, indicating the capability to capture 64000 data points per second, surpassing the capabilities of ADC-based microphones.

DMA (Direct Memory Access) enables external devices to access system memory directly without CPU intervention. This enhances data transfer efficiency, reduces CPU burden, and ultimately improves overall system performance. To illustrate the relationship between DMA and the MCU, let's use an analogy: DMA is like a self-reliant child who, after completing one "assignment," reports to their parent (CPU/MCU) and then independently moves on to another "assignment" while the parent is busy checking the previous one. In this context, the dma_buf_count parameter should be at least 2, analogous to assigning two tasks to the child; otherwise, if the child finishes one task, they will have nothing to do and might get distracted. It can also be set to 3, adding an extra assignment to keep the child engaged.

As for dma_buf_len, its significance lies in ensuring that the child completes a whole page of assignments before seeking the parent's attention. Setting this parameter as large as possible within the range of 8-1024 minimizes the frequency and duration of CPU occupation, allowing the child to work in harmony with the parent, resulting in a pleasant and efficient completion of tasks.
```c
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
```


### FFT Data Processing

On the surface, we only see two entry functions. The first step involves creating an audio data processing task on the second core of the esp32 using FreeRTOS. This task is represented by the handle `processing_task_handle`. The second function is `i2s_sampler.start`, where we pass the previously configured i2s settings to it, and it silently performs its work in the background, releasing the DMA-captured data through the aforementioned handle.

Inside the I2SSampler, all the classic techniques of FreeRTOS are employed, including message queues, direct message notifications, and multitask creation. While it may seem chaotic, in reality, it's bustling with activity to "prepare meals" for the bosses behind the scenes.

Another hero in this setup is the Processor, embodying the FFT. It receives messages from the I2SSampler, then processes and transforms them into clean and beautiful data. I admire such diligent colleagues who work quietly and selflessly, doing good deeds without seeking recognition. In reality, these unsung heroes working diligently in the background are the essence of this project.

```c
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

```
### LVGL Dynamic Drawing

We employ an amplitude update function (bar_value_update) to handle the spectrum and screen size adaptation. This function takes a float array mag obtained from the Fourier transform calculation. It iterates through every other element of the mag array, calculates their average value ave, compares it with the window height, and derives the bar chart value. The overall logic involves computing the weighted average, updating the bar chart value and peak based on conditions, and applying first-order lag smoothing for smoother bar chart transitions.

The crucial aspect of graphic drawing lies in treating the spectrum object as a container for drawing (spectrum_draw_event_cb). Instead of using LVGL's conventional pre-set drawing functions, the drawing is accomplished using low-level drawing methods. Actual drawing occurs during the LV_EVENT_DRAW_POST (after drawing) event, where operations are performed to draw the spectrum object. The process involves using mechanisms such as drawing rectangles (lv_draw_rect) and drawing lines (lv_draw_line). The rectangles represent the instantaneous spectrum intensity, while the lines represent the lag response of the spectrum peak. Due to the 2-pixel width of the lines, they appear as small rectangular blocks. By iterating through an array of size SAMPLE_SIZE, which represents the spectrum analysis result data, the function draws rectangles and two lines. The arrays bar_chart and bar_chart_peaks are used to determine the bar chart heights. The bar chart is dynamically drawn on the spectrum_obj object, and its height data is continuously updated in real time. This creates a dynamic effect that is even responsive and effective on monochrome OLED screens, producing impressive visual dynamics.

```c
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
```

### Summary:

The xiao_expansion_board's display showcases music information processed through FFT, providing instant feedback on the sound captured by the microphone. It's like a miniature musical fountain, where the kinetic energy of the notes springs up from the horizontal plane, creating a spectacular scene of beautiful splashing water. Each frequency component is akin to splashing water, revealing the colorful details and textures of the music, from the stable and mellow low frequencies to the lively and agile high frequencies. Each note exhibits unique tension and resilience.

This effectively demonstrates the charm and magic of FFT in analyzing music. This approach seemingly allows one to perceive the soul of the music through the spectral data, creating an enchanting experience.





### reference

1. [m5stack-core2-audio-monitor](https://github.com/atomic14/m5stack-core2-audio-monitor)

    Thanks atomic14's awesome demo, The code for audio and FFT processing is referenced from this project.

2. [lv_demo_music](https://github.com/lvgl/lv_demos/tree/master/src/lv_demo_music)
   
   Spectrum graphics drawing method reference this demo.


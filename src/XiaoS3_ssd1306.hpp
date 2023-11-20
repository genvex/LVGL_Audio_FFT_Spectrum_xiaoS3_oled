#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

//从 lgfx_device 派生并创建一个进行自己设置的类。
class SSD1306 : public lgfx::LGFX_Device
{
lgfx::Panel_SSD1306     _panel_instance;
lgfx::Bus_I2C       _bus_instance;   // I2C总线的实例(仅ESP32)

public:
 
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

    { // 进行显示面板控制的设置。
      auto cfg = _panel_instance.config();    // 取得显示面板设定用的构造体。
      cfg.pin_cs           =    -1;  // CS连接的引脚编号(-1=disable)
      cfg.pin_rst          =    -1;  // RST连接的引脚编号(-1=disable)
      cfg.pin_busy         =    -1;  // BUSY连接的引脚编号(-1=disable)

      cfg.memory_width     =   128;  
      cfg.memory_height    =    64;  
      cfg.panel_width      =   128;  
      cfg.panel_height     =    64;  
      cfg.offset_x         =    0;  
      cfg.offset_x         =    0; 
      cfg.offset_y         =    0;  
      cfg.offset_rotation  =     2;  
      cfg.dummy_read_pixel =     8;  
      cfg.dummy_read_bits  =     1;  
      cfg.readable         = false; 
      cfg.invert           = false;  
      cfg.rgb_order        = false;  
      cfg.dlen_16bit       = false;  
      cfg.bus_shared       = false;  
      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance); // 设置要使用的面板。
  }
};

#pragma once

#define LGFX_USE_V1

#include <LovyanGFX.hpp>

// 在ESP32中单独设置使用LovyanGFX时的设置示例

/*
//ESP 32中独立设定使用 lovyangfx 时的设定示例请复制这个文件，给它一个新的名字，并根据您的环境更改您的设置。
创建的文件可以从用户程序中包含。
复制的文件可以放在图书馆的 lgfx_user 文件夹中使用，但是/n请注意，这可能会在库更新时被删除。
如果您希望安全运行，请创建备份或将其放在用户项目的文件夹中。

//*/

///从 lgfx_device 派生并创建一个进行自己设置的类。
class Pyclock : public lgfx::LGFX_Device
{
/*
您可以将类名称从“LGFX”更改为另一个名称。
如果与AUTODETECT配合使用，请将其更名为LGFX以外的名称，因为使用的是“LGFX”。
另外，如果同时使用多个面板，请为每个面板命名不同的名称。
※更改类名称时，构造函数的名称也必须更改为相同的名称。
名字的命名方法可以自由决定，但设想设定增加的情况，
例如，在ESP32DevKit-C中进行SPI连接的ILI 9341的设定的情况下，
LGFX_DevKitC_SPI_ILI 9341。
像这样的名字，让文件名和类名一致，在使用时变得迷茫不了。
//*/


//提供与要连接的面板类型相匹配的实例。

//lgfx::Panel_ILI9341     _panel_instance;
//lgfx::Panel_SH110x      _panel_instance; // SH1106, SH1107
//  lgfx::Panel_SSD1306     _panel_instance;
lgfx::Panel_ST7789      _panel_instance;


//提供与要连接的面板类型相匹配的实例。
  lgfx::Bus_SPI       _bus_instance;   // SPI总线的实例
//lgfx::Bus_I2C       _bus_instance;   // I2C总线的实例(仅ESP32)
//lgfx::Bus_Parallel8 _bus_instance;   // 8ビットパラレルバスのインスタンス (ESP32のみ)


//如果可以进行背光控制，则提供实例。(如无必要删除)
//  lgfx::Light_PWM     _light_instance;

//提供与触摸屏类型匹配的实例。(如无必要删除)
//  lgfx::Touch_FT5x06      _touch_instance; // FT5206, FT5306, FT5406, FT6206, FT6236, FT6336, FT6436
//lgfx::Touch_GT911       _touch_instance;
//lgfx::Touch_STMPE610    _touch_instance;
//lgfx::Touch_XPT2046     _touch_instance;

public:

  //创建构造函数，在这里进行各种设置。如果你改变了类的名字，构造函数也应该指定相同的名字。
  Pyclock(void)
  {
    { //进行总线控制的设置。
      auto cfg = _bus_instance.config();  // 获取总线设置的结构。

// SPI设置
      //cfg.spi_host = VSPI_HOST;     // 选择要使用的SPI (vspi_host or hspi_host)
      //cfg.spi_host = HSPI_HOST;     // 选择要使用的SPI (vspi_host or hspi_host)
      cfg.spi_host = SPI2_HOST; 
      cfg.spi_mode = 0;             // 设置SPI通信模式(0到3)
/*
HSPI和VSPI并不是网友们认为的high-speed SPI 和Very High-speed SPI，HSPI、VSPI是一样的，
只不过是换个名字用于区分，SPI相当于SPI0或SPI1，HSPI相当于SPI2，VSPI相当于SPI3。
ESP32 共有 4 个 SPI 控制器 SPI0、SPI1、SPI2、SPI3，用于连接支持 SPI 协议的设备。
SPI0 控制器作为 cache 访问外部存储单元接口使用。
SPI1 作为主机使用。
SPI2 和 SPI3 控制器既可作为主机使用又可作为从机使用。作主机使用时，每个 SPI 控制器可以使用多个片选信号 (CS0 ~ CS2) 来连接多个 SPI 从机设备。
SPI1 ~ SPI3 控制器共享两个 DMA 通道。
*/
      cfg.freq_write = 40000000;    // 发送时的SPI时钟(最大80 MHz，80 MHz四舍五入为整数)
      cfg.freq_read  = 16000000;    // 接收时的SPI时钟
      cfg.spi_3wire  = true;        // 用MOSI引脚进行接收时，设定为true。
      cfg.use_lock   = true;        // 如果使用事务锁定，则设置true
      cfg.dma_channel = 1;          // 设置DMA通道(1or 2.0=disable)设置要使用的DMA通道(0=不使用DMA)
      cfg.pin_sclk = 40;            // 设置 SPI 的 sclk 引脚编号
      cfg.pin_mosi = 41;            // 设定 SPI 的 mosi 引脚编号
      cfg.pin_miso = -1;            // 设置SPI的MISO引脚编号(-1=disable)
      cfg.pin_dc   = 38;            // 设置SPI的D/C引脚编号(-1 = disable)
     

     // 使用与 sd 卡共用的 SPI 总线时，请不要省略 miso，一定要设置 miso。

/* I2C设置
      cfg.i2c_port    = 0;          //  (0 or 1)
      cfg.freq_write  = 400000;     // 写速
      cfg.freq_read   = 400000;     // 收速
      cfg.pin_sda     = 21;         // SDA引脚编号
      cfg.pin_scl     = 22;         // SCL引脚编号
      cfg.i2c_addr    = 0x3C;       // I2C地址
*/

      _bus_instance.config(cfg);    // 将设定值反映在总线上。
      _panel_instance.setBus(&_bus_instance);      // 把总线设置在面板上。
    }

    { // 进行显示面板控制的设置。
      auto cfg = _panel_instance.config();    // 取得显示面板设定用的构造体。
      cfg.pin_cs           =    39;  // CS连接的引脚编号(-1=disable)
      cfg.pin_rst          =    42;  // RST连接的引脚编号(-1=disable)
      cfg.pin_busy         =    -1;  // BUSY连接的引脚编号(-1=disable)

      // ※ 以下的设定值是根据每个面板设定的一般的初始值，不清楚的项目请试着删除。

      cfg.memory_width     =   240;  // 驱动IC所支持的最大宽度
      cfg.memory_height    =   240;  // 驱动IC所支持的最大高度
      cfg.panel_width      =   240;  // 实际可显示的宽度
      cfg.panel_height     =   240;  // 实际可显示的高度
      cfg.offset_x         =    0;  // 面板的X方向偏移量
      cfg.offset_y         =    0;  // 面板的Y方向偏移量
      cfg.offset_rotation  =     0;  // 旋转方向上的值的偏移0到7(4到7是上下反转)
      cfg.dummy_read_pixel =     8;  // 像素读取前虚拟读取的位数
      cfg.dummy_read_bits  =     1;  // 像素以外的数据读取前的虚拟读取的位数
      cfg.readable         = true;  // 如果可以读取数据则设置为 true
      cfg.invert           = true;  // 面板的明暗反转的情况下设定为真
      cfg.rgb_order        = false;  // 如果面板中的红色和蓝色被替换，则设置为true
      cfg.dlen_16bit       = false;  // 在以16位为单位发送数据长度的面板的情况下，设定为true
      cfg.bus_shared       = true;  // 如果您与SD卡共享总线，则设置为true(通过drawJpgFile等进行总线控制)

      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance); // 设置要使用的面板。
  }
};


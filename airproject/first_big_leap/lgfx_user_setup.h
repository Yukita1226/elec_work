#pragma once

#ifndef ESP32S3
  #define ESP32S3
#endif

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_RGB     _panel_instance;
  lgfx::Bus_RGB       _bus_instance;
  lgfx::Light_PWM     _light_instance;

public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.panel = &_panel_instance;
      
      cfg.pin_d0  = 14; 
      cfg.pin_d1  = 38; 
      cfg.pin_d2  = 18; 
      cfg.pin_d3  = 17; 
      cfg.pin_d4  = 10; 
      
      cfg.pin_d5  = 39; 
      cfg.pin_d6  = 0;  
      cfg.pin_d7  = 45; 
      cfg.pin_d8  = 48; 
      cfg.pin_d9  = 47; 
      cfg.pin_d10 = 21; 
      
      cfg.pin_d11 = 1;  
      cfg.pin_d12 = 2;  
      cfg.pin_d13 = 42; 
      cfg.pin_d14 = 41; 
      cfg.pin_d15 = 40; 

      cfg.pin_henable = 5;  
      cfg.pin_vsync   = 3;  
      cfg.pin_hsync   = 46; 
      cfg.pin_pclk    = 7;  

      cfg.freq_write  = 16000000; 

      cfg.hsync_polarity    = 0;
      cfg.hsync_front_porch = 40;
      cfg.hsync_pulse_width = 48;
      cfg.hsync_back_porch  = 88;
      
      cfg.vsync_polarity    = 0;
      cfg.vsync_front_porch = 13;
      cfg.vsync_pulse_width = 3;
      cfg.vsync_back_porch  = 32;
      
      cfg.pclk_idle_high    = 0;
      
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.memory_width  = 800;
      cfg.memory_height = 480;
      cfg.panel_width   = 800;
      cfg.panel_height  = 480;
      cfg.offset_x      = 0;
      cfg.offset_y      = 0;
      _panel_instance.config(cfg);
    }

    {
      auto cfg = _panel_instance.config_detail();
      cfg.use_psram = 1; 
      _panel_instance.config_detail(cfg);
    }

    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = 2; 
      cfg.pwm_channel = 1;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    setPanel(&_panel_instance);
  }
};
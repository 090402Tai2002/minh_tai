#include <Arduino.h>
#include <Adafruit_BusIO_Register.h>
#include <Wire.h>
#include <EEPROM.h>
#include "HX711.h"
#include <ErriezRotaryFullStep.h>
#include "esp_mac.h"
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
/* TAG OUT */

// DONGG CO
#define ESC 13
//
#define DD_RUNG 27

#define DC_XA 14
/* TAG IN */
// ENCODER oke ko chinh
#define ENCODER_A 33
#define ENCODER_B 25
#define BUTTON 32
// CAN
// HX711 circuit wiring

// CAM BIEN HET THUC
#define HET_THUC 15

namespace main
{
    namespace display
    {
        struct TIME // lưu thời gian
        {
        public:
            uint8_t h; 
            uint8_t m;
            uint8_t s; 
        };

        extern TIME time_on;
        extern TIME time_off;
        extern uint32_t mat_do;
        extern uint32_t so_lan_phun;
        extern uint32_t cycle_on;
        extern uint32_t cycle_off;
        extern uint32_t min_on;  // so lan phun
        extern uint32_t sec_off; // so lan phun
        extern uint16_t so_can_cali;
        extern uint16_t van_toc_cao;
        extern uint16_t van_toc_thap;
        extern bool state_mode;
        extern int MAX_WISE;
        extern int MIN_WISE;
        extern volatile uint32_t count;
        extern volatile uint32_t countLast;
        extern LiquidCrystal_I2C lcd;
        extern unsigned long time_check;
        extern char run_mode[4][20];
        extern int on_time_seconds[10];
        extern int off_time_seconds[10];
        // extern unsigned long tick_press, tick_time_press, press_time;
        void setup();
        void setup_wifi(String ssid, String pass);
        void mode_setup(bool status);
        void run_mode_diplay();
        void menu_task_1();
        void menu_task_2();
        void setting_state();
        void display_weigh(float weigh);

    }

    namespace action
    {
            enum state
        {
            STOP,
            RUN
        };
        extern state action_state_auto, action_state_manual;
        void rotaryInterrupt();
        void setup();
        void read_status();
    }

    namespace escct
    {

        void setup();
        void control_speed(uint16_t speed);
        void pid_control(float so_ky_cho_an);
        void bat_dau_chu_ky_cho_an();
        void tat_tat_ca_thiet_bi();

    }
    namespace can
    {
        extern HX711 scale;
        void setup();
        void read();
        void calibration();
    }
    namespace save_data
    {
        // Đối tượng Preferences để thao tác với bộ nhớ không bay hơi
        extern Preferences preferences;
        void saveData(); // Hàm lưu dữ liệu
        void loadData(); // Hàm đọc dữ liệu

    }
    namespace wifi
    {
        extern WiFiClient espClient;
        extern PubSubClient mqttClient;
        void sendata_mqtt(int value);
        bool setup();
        void main();
        void main_setup();
        void save_wifi(const String &ssid, const String &pass);

    }
    namespace time_set
    {
        struct timer_set
        {
            uint8_t hour;
            uint8_t min;
            uint8_t sec;
            /* data */
        };

        extern timer_set time_set_on;
        extern timer_set time_set_off;

        extern RTC_DS1307 rtc;
        extern DateTime now;

        extern uint8_t now_secon;
        extern uint8_t last_secon;
        void cacula_time_on(uint8_t hou, uint8_t min, uint8_t sec);
        void cacula_time_off(uint8_t hou, uint8_t min, uint8_t sec);
        void setup();
        void read_time();
    }

}
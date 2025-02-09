#include "lib.h"
namespace main
{

    namespace action
    {

        state action_state_auto = STOP, action_state_manual = STOP;
        unsigned long tick_press, tick_time_press, press_time = 15;
        bool mode = false;
        const uint8_t chu_ky_cho_an = 60;
        RotaryFullStep rotary(ENCODER_A, ENCODER_B);
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
        ICACHE_RAM_ATTR
#endif

        void rotaryInterrupt()
        {
            if (display::state_mode == true)
            {
                int rotaryState;
                rotaryState = rotary.read();

                if (rotaryState == 0)
                {
                    return;
                }
                else if (abs(rotaryState) >= 2)
                {
                    display::count += rotaryState * 2;
                }
                else
                {
                    display::count += rotaryState;
                }
                if (display::count > display::MAX_WISE)
                {
                    display::count = display::MAX_WISE;
                }
                if (display::count < display::MIN_WISE)
                {
                    display::count = display::MIN_WISE;
                }
                if (display::countLast != display::count)
                {
                    display::countLast = display::count;
                }
            }
        }
        void setup()
        {

            pinMode(DD_RUNG, OUTPUT);
            pinMode(DC_XA, OUTPUT);
            pinMode(ENCODER_A, INPUT_PULLUP);
            pinMode(ENCODER_B, INPUT_PULLUP);
            pinMode(BUTTON, INPUT);

            Serial.begin(115200);
            display::setup();
            // escct::setup();
            wifi::main_setup();
            time_set::setup();
            can::setup();
            save_data::loadData();
            time_set::read_time();
            time_set::cacula_time_on(0, 0, 30);
            rotary.setSensitivity(10);
            detachInterrupt(ENCODER_A);
            detachInterrupt(ENCODER_B);
        }
        void read_status()
        {
            while (true)
            {
                if (digitalRead(BUTTON) == 0 && mode == false)
                {
                    tick_press = millis();
                    mode = true;
                }
                if (mode == true)
                {
                    tick_time_press = (uint32_t)millis();
                }
                if (tick_time_press - tick_press > press_time && mode == true)
                {
                    attachInterrupt(digitalPinToInterrupt(ENCODER_A), rotaryInterrupt, CHANGE);
                    attachInterrupt(digitalPinToInterrupt(ENCODER_B), rotaryInterrupt, CHANGE);
                    escct::tat_tat_ca_thiet_bi();
                    while (digitalRead(BUTTON) == 0)
                    {
                    };
                    
                    mode = false;
                    tick_press = 0;
                    save_data::loadData();
                    display::setting_state();
                    int hours = display::off_time_seconds[display::cycle_off] / 3600;          // Tính số giờ
                    int minutes = (display::off_time_seconds[display::cycle_off] % 3600) / 60; // Tính số phút
                    int seconds = display::off_time_seconds[display::cycle_off] % 60;
                    time_set::cacula_time_on(hours, minutes, seconds);
                }
                if ((uint32_t)millis() - tick_press > press_time && mode == true)
                {
                    mode = false;
                    tick_press = 0;
                    display::state_mode = false;
                }

                if (display::state_mode == false)
                {
                    time_set::read_time();
                    can::read();
                    if (time_set::now_secon != time_set::last_secon)
                    {
                        display::run_mode_diplay();
                        time_set::now_secon = time_set::last_secon;
                        if (time_set::time_set_on.hour == time_set::now.hour() && time_set::time_set_on.min == time_set::now.minute() && time_set::time_set_on.sec == time_set::now.second() && action::action_state_auto == STOP)
                        {
                            action::action_state_auto = RUN;
                            escct::bat_dau_chu_ky_cho_an();
                            Serial.println(" on ");
                        }
                        time_set::now_secon = time_set::last_secon;
                    }
                }
            }
        }
    }
}

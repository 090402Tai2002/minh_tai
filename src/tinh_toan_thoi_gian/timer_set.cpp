#include "lib.h"
namespace main
{

    namespace time_set
    {
        RTC_DS1307 rtc;
        DateTime now;

        uint8_t now_secon, last_secon = 0;
        char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

        timer_set time_set_on;
        timer_set time_set_off;

        void cacula_time_on(uint8_t hou, uint8_t min, uint8_t sec)
        {

            DateTime time_on(now + TimeSpan(0, hou, min, sec));

            time_set_on.hour = time_on.hour();
            time_set_on.min = time_on.minute();
            time_set_on.sec = time_on.second();

            Serial.print(time_set_on.hour);
            Serial.print(':');
            Serial.print(time_set_on.min);
            Serial.print(':');
            Serial.print(time_set_on.sec);
            Serial.println();

            snprintf(display::run_mode[2], 20, "CYCLE:  %02d:%02d:%02d   ", time_on.hour(), time_on.minute(), time_on.second());
        }

        void cacula_time_off(uint8_t hou, uint8_t min, uint8_t sec)
        {
            now = rtc.now();
            DateTime time_on(now + TimeSpan(0, hou, min, sec));
            time_set_off.hour = time_on.hour();
            time_set_off.min = time_on.minute();
            time_set_off.sec = time_on.second();
        }
        void syn_time_internet()
        {
        }
        void setup()
        {

            if (!rtc.begin())
            {
                Serial.println("Couldn't find RTC");
                Serial.flush();
                while (1)
                    delay(10);
            }
            if (rtc.isrunning())
            {
                rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
                Serial.println("RTC is NOT running, let's set the time!");
            }
        }

        void read_time()
        {
            now = rtc.now();
            now_secon = now.second();
            snprintf(display::run_mode[0], 20, "TIME :  %02d:%02d:%02d   ", time_set::now.hour(), time_set::now.minute(), time_set::now.second());
        }

    }
}
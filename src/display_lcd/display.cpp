#include "lib.h"

namespace main
{
    namespace display
    {

        // Thời gian Run: Có thể cách nhau 30p (VD: 00h, 0h30p, 1h30p)
        // Thời gian ON (phun): Cách nhau 5s ( VD: 5s, 10s, 15s)
        // Thời gian OFF( nghỉ): 1p, 3p, 5p, 10p, 15p, 20p, 30p, 1h, 2h.

        LiquidCrystal_I2C lcd(0x27, 20, 4);

        byte tick_char[] = {
            B11000,
            B11100,
            B11110,
            B11111,
            B11111,
            B11110,
            B11100,
            B11000,
        };

        int MAX_WISE = 4;
        int MIN_WISE = 0;

        unsigned long time_check;
        bool state_mode;
        volatile uint32_t count = 0;
        volatile uint32_t countLast = 0;

        TIME time_on;
        TIME time_off;
        uint32_t cycle_on;  // so lan phun
        uint32_t cycle_off; // so lan phun
        uint32_t min_on;    // so lan phun
        uint32_t sec_off;   // so lan phun
        uint32_t mat_do;
        uint32_t so_lan_phun;
        uint16_t so_can_cali;
        uint16_t van_toc_cao;
        uint16_t van_toc_thap;

        enum State
        {
            MAIN_MENU_1,
            MAIN_MENU_2,
            MAT_DO,
            THOI_GIAN_PHUN,
            THOIGIANPHUNNGHI,
            VAN_TOC,
            NHAP_CAN_CALI,
            HIEU_CHAN_CAN,
            SO_KY_CHO_AN,
            EXIT
        };

        State currentState = MAIN_MENU_1;

        char menuItems_1[4][20] = {
            "MAT DO             ",
            "THOI GIAN CHAY     ",
            "THOI GIAN PHUN-NGHI",
            "HOME----------NEXT ",
        };

        char menuItems_2[4][20] = {
            "VAN TOC            ",
            "HIEU CHUAN CAN     ",
            "SO KY CHO AN       ",
            "BACK-----------END",
        };

        char time_task[4][20] = {
            "TIME OFF:    :    ",
            "TIME ON :    :    ",
            "---------------BACK",
            "                  ",
        };
        char thoi_gian_phun_nghi[4][20] =
            {
                "TIME PHUN:         ",
                "TIME NGHI:         ",
                "---------------BACK",
                "                   ",
        };
        char set_van_toc[4][20] =
            {
                "HIGH    :         %",
                "LOW     :         %",
                "---------------BACK",
                "                  ",
        };

        char load_cell_task[4][20] = {
            "SO KG   :       KG",
            "CALI.B  :      YES",
            "OUT------------BACK",
            "                   ",
        };
        char so_ky_cho_an[4][20] = {
            "SO KG   :       KG",
            "CALI.B  :      YES",
            "OUT------------BACK",
            "                   ",
        };

        char run_mode[4][20] = {
            "TIME :             ",
            "VALUE:             ",
            "CYCLE:             ",
            "WEIGH:             ",
        };

        char off_time[10][10] =
            {
                "30SEC    ",
                "1MIN     ",
                "3MIN     ",
                "5MIN     ",
                "10MIN    ",
                "15MIN    ",
                "20MIN    ",
                "30MIN    ",
                "1HOUR    ",
                "2HOUR    "};
        char on_time[10][10] =
            {
                "5SEC     ",
                "10SEC    ",
                "15SEC    ",
                "20SEC    ",
                "25SEC    ",
                "30SEC    ",
                "35SEC    ",
                "40SEC    ",
                "45SEC    ",
                "50SEC    "};

        int on_time_seconds[10] = {5, 10, 15, 20, 25, 30, 35, 40, 45, 50};
        int off_time_seconds[10] =
            {
                30,   // 30SEC
                60,   // 1MIN
                180,  // 3MIN
                300,  // 5MIN
                600,  // 10MIN
                900,  // 15MIN
                1200, // 20MIN
                1800, // 30MIN
                3600, // 1HOUR
                7200  // 2HOUR
        };
        void menu_task_2(bool status);
        void nhap_so_can(bool status);
        void display_weigh(float can_nang);
        void clear_clow(int clow);
        void display_weigh(float can_nang)
        {
            snprintf(run_mode[3], sizeof(run_mode[3]), "WEIGH:  %.1fgram     ", can_nang);
        }
        void setup()
        {
            lcd.init();
            lcd.backlight();
            lcd.createChar(0, tick_char);
        }

        void run_mode_diplay()
        {
            for (int i = 0; i < 4; ++i)
            {
                lcd.setCursor(1, i);
                lcd.print(run_mode[i]);
            }
        }
        void clear_clow(int clow)
        {
            for (int i; i < 4; ++i)
            {
                lcd.setCursor(clow, i);
                lcd.print(" ");
            }
        }
        void setup_wifi(String ssid, String pass)
        {
            lcd.setCursor(1, 0);
            lcd.print("MAY CHO TOM AN  ");
            lcd.setCursor(1, 1);
            lcd.print("CONECTING WIFI ");
            lcd.setCursor(1, 2);
            lcd.print(ssid);
            lcd.setCursor(1, 3);
            lcd.print(pass);
        }
        void set_time_on(bool status)
        {
            unsigned long tick_press, tick_time_press, press_time = 15;
            bool mode = false;
            tick_press = 0;
            volatile int step = 0;
            volatile int last_step = 0;
            clear_clow(0);
            int row = 1;
            while (status == true)
            {
                lcd.setCursor(10, 3);
                lcd.print(step);

                if (digitalRead(BUTTON) == 0 && mode == false)
                {
                    tick_press = millis();
                    mode = true;
                }
                if (mode == true)
                    tick_time_press = (uint32_t)millis();

                if (tick_time_press - tick_press > press_time && mode == true)
                {
                    while (digitalRead(BUTTON) == 0)
                    {
                    };
                    step = step + 1;
                }
                if (((uint32_t)millis()) - tick_press > press_time && mode == true)
                {
                    mode = false;
                    tick_press = 0;
                }
                if (last_step != step)
                {
                    if (step == 1)
                    {
                        lcd.setCursor(10, row);
                        lcd.print("->");
                        lcd.setCursor(18, row);
                        lcd.print("  ");
                        last_step = step;
                        MAX_WISE = 24;
                        MIN_WISE = 0;
                    }
                    if (step == 2)
                    {
                        lcd.setCursor(10, row);
                        lcd.print("  ");
                        lcd.setCursor(17, row);
                        lcd.print("<-");
                        MAX_WISE = 60;
                        MIN_WISE = 0;
                        last_step = step;
                    }
                    if (step == 3)
                    {
                        lcd.setCursor(10, row);
                        lcd.print("  ");
                        lcd.setCursor(17, row);
                        lcd.print("  ");
                        last_step = step;
                    }
                }
                if (step == 1)
                {
                    time_on.h = count;
                    if (time_on.h > MAX_WISE)
                        time_on.h = MAX_WISE;
                    lcd.setCursor(12, row);
                    if (time_on.h < 10)
                        lcd.print("0");
                    lcd.print(time_on.h);
                }
                if (step == 2)
                {
                    time_on.m = count;
                    if (time_on.m > MAX_WISE)
                        time_on.m = MAX_WISE;
                    lcd.setCursor(15, row);
                    if (time_on.m < 10)
                        lcd.print("0");
                    lcd.print(time_on.m);
                }
                if (step == 3)
                {
                    MAX_WISE = 2;
                    MIN_WISE = 0;
                    currentState = THOI_GIAN_PHUN;
                    break;
                }
            }
        }
        void set_time_off(bool status)
        {
            unsigned long tick_press, tick_time_press, press_time = 15;
            volatile int step = 0;
            volatile int last_step = 0;
            clear_clow(0);
            int row = 0;
            bool mode = false;
            tick_press = 0;
            while (status == true)
            {
                lcd.setCursor(10, 3);
                lcd.print(step);
                if (digitalRead(BUTTON) == 0 && mode == false)
                {
                    tick_press = millis();
                    mode = true;
                }
                if (mode == true)
                    tick_time_press = (uint32_t)millis();

                if (tick_time_press - tick_press > press_time && mode == true)
                {
                    while (digitalRead(BUTTON) == 0)
                    {
                    };
                    step = step + 1;
                }
                if (((uint32_t)millis()) - tick_press > press_time && mode == true)
                {
                    mode = false;
                    tick_press = 0;
                }
                if (last_step != step)
                {
                    if (step == 1)
                    {
                        lcd.setCursor(10, row);
                        lcd.print("->");
                        lcd.setCursor(18, row);
                        lcd.print("  ");
                        last_step = step;
                        MAX_WISE = 24;
                        MIN_WISE = 0;
                    }
                    if (step == 2)
                    {
                        lcd.setCursor(10, row);
                        lcd.print("  ");
                        lcd.setCursor(17, row);
                        lcd.print("<-");
                        MAX_WISE = 60;
                        MIN_WISE = 0;
                        last_step = step;
                    }
                    if (step == 3)
                    {
                        lcd.setCursor(10, row);
                        lcd.print("  ");
                        lcd.setCursor(17, row);
                        lcd.print("  ");
                        last_step = step;
                    }
                }
                if (step == 1)
                {
                    time_off.h = count;
                    if (time_off.h > MAX_WISE)
                        time_off.h = MAX_WISE;
                    lcd.setCursor(12, row);
                    if (time_off.h < 10)
                        lcd.print("0");
                    lcd.print(time_off.h);
                }
                if (step == 2)
                {
                    time_off.m = count;
                    lcd.setCursor(15, row);
                    if (time_off.m > MAX_WISE)
                        time_off.m = MAX_WISE;
                    if (time_off.m < 10)
                        lcd.print("0");
                    lcd.print(time_off.m);
                }
                if (step == 3)
                {
                    MAX_WISE = 2;
                    MIN_WISE = 0;
                    currentState = THOI_GIAN_PHUN;
                    break;
                }
            }
        }
        void thoi_gian_phun(bool status)
        {
            unsigned long tick_press, tick_time_press, press_time = 15;
            volatile int step = 0;
            volatile int last_step = 0;
            clear_clow(0);
            int row = 0;
            bool mode = false;
            tick_press = 0;
            while (status == true)
            {
                lcd.setCursor(10, 3);
                lcd.print(step);
                if (digitalRead(BUTTON) == 0 && mode == false)
                {
                    tick_press = millis();
                    mode = true;
                }
                if (mode == true)
                    tick_time_press = (uint32_t)millis();

                if (tick_time_press - tick_press > press_time && mode == true)
                {
                    while (digitalRead(BUTTON) == 0)
                    {
                    };
                    step = step + 1;
                }
                if (((uint32_t)millis()) - tick_press > press_time && mode == true)
                {
                    mode = false;
                    tick_press = 0;
                }

                if (last_step != step)
                {
                    if (step == 1)
                    {
                        last_step = step;
                        MAX_WISE = 5;
                        MIN_WISE = 0;
                    }
                }

                if (step == 1)
                {
                    lcd.setCursor(11, row);
                    lcd.print(on_time[count]);
                    cycle_on = count;
                }

                if (step == 2)
                {
                    MAX_WISE = 2;
                    MIN_WISE = 0;
                    currentState = THOIGIANPHUNNGHI;
                    break;
                }
            }
        }
        void thoi_gian_nghi(bool status)
        {
            unsigned long tick_press, tick_time_press, press_time = 15;
            volatile int step = 0;
            volatile int last_step = 0;
            clear_clow(0);
            int row = 1;
            bool mode = false;
            tick_press = 0;
            while (status == true)
            {
                lcd.setCursor(10, 3);
                lcd.print(step);
                if (digitalRead(BUTTON) == 0 && mode == false)
                {
                    tick_press = millis();
                    mode = true;
                }
                if (mode == true)
                    tick_time_press = (uint32_t)millis();

                if (tick_time_press - tick_press > press_time && mode == true)
                {
                    while (digitalRead(BUTTON) == 0)
                    {
                    };
                    step = step + 1;
                }
                if (((uint32_t)millis()) - tick_press > press_time && mode == true)
                {
                    mode = false;
                    tick_press = 0;
                }

                if (last_step != step)
                {
                    if (step == 1)
                    {
                        last_step = step;
                        MAX_WISE = 9;
                        MIN_WISE = 0;
                    }
                }

                if (step == 1)
                {
                    lcd.setCursor(11, row);
                    lcd.print(off_time[count]);
                    cycle_off = count;
                }

                if (step == 2)
                {
                    MAX_WISE = 2;
                    MIN_WISE = 0;
                    currentState = THOIGIANPHUNNGHI;
                    break;
                }
            }
        }
        void cau_hinh_thoi_gian(bool status) // cái này thafh 2 task
        {
            MAX_WISE = 2;
            MIN_WISE = 0;
            bool mode = false;
            unsigned long tick_press, tick_time_press, press_time = 10;
            clear_clow(0);
            int row = 0;
            lcd.clear();

            snprintf(thoi_gian_phun_nghi[0], sizeof(thoi_gian_phun_nghi[0]), "TIME PHUN: %s", off_time[cycle_off]);
            snprintf(thoi_gian_phun_nghi[1], sizeof(thoi_gian_phun_nghi[1]), "TIME NGHI: %s", on_time[cycle_on]);

            volatile int last_count = 0;

            for (int i = 0; i < 4; ++i)
            {
                lcd.setCursor(1, i);
                lcd.print(thoi_gian_phun_nghi[i]);
            }
            while (status == true)
            {

                if (last_count != count)
                {
                    lcd.setCursor(0, last_count);
                    lcd.print(" ");
                    lcd.setCursor(0, count);
                    lcd.write(0);
                    lcd.setCursor(10, 3);
                    lcd.print(count);
                    last_count = count;
                }
                if (digitalRead(BUTTON) == 0 && mode == false)
                {
                    tick_press = millis();
                    mode = true;
                }
                if (mode == true)
                    tick_time_press = (uint32_t)millis();

                if (tick_time_press - tick_press > press_time && mode == true)
                {
                    if (digitalRead(BUTTON) == 0 && count == 0)
                    {
                        thoi_gian_phun(true);
                    }
                    if (digitalRead(BUTTON) == 0 && count == 1)
                    {
                        thoi_gian_nghi(true);
                    }
                    if (digitalRead(BUTTON) == 0 && count == 2)
                    {
                        MAX_WISE = 4;
                        MIN_WISE = 0;
                        currentState = MAIN_MENU_1;
                        status = false;
                        break;
                    }
                }
                if (((uint32_t)millis()) - tick_press > press_time && mode == true)
                {
                    mode = false;
                    tick_press = 0;
                }
            }
        }
        void set_mat_do(bool status)
        {

            volatile int step = 0;
            volatile int last_step = 0;
            unsigned long tick_press, tick_time_press, press_time = 15;
            clear_clow(0);
            int row = 0;
            count = mat_do / 1000;
            bool mode = false;
            while (status == true)
            {

                if (digitalRead(BUTTON) == 0 && mode == false)
                {
                    tick_press = millis();
                    mode = true;
                }
                if (mode == true)
                    tick_time_press = (uint32_t)millis();

                if (tick_time_press - tick_press > press_time && mode == true)
                {
                    while (digitalRead(BUTTON) == 0)
                    {
                    };
                    step = step + 1;
                }
                if (((uint32_t)millis()) - tick_press > press_time && mode == true)
                {
                    mode = false;
                    tick_press = 0;
                }
                if (last_step != step)
                {
                    if (step == 1)
                    {
                        lcd.setCursor(11, row);
                        lcd.print("-->");
                        last_step = step;
                        MAX_WISE = 300;
                        MIN_WISE = 0;
                    }

                    if (step == 2)
                    {
                        lcd.setCursor(11, row);
                        lcd.print("   ");
                    }
                }
                if (step == 1)
                {
                    mat_do = count * 1000;
                    if (count > MAX_WISE)
                        mat_do = count * 1000;
                    lcd.setCursor(14, row);
                    if (mat_do <= 9000)
                        lcd.print("00");
                    if (mat_do > 9000 && mat_do <= 90000)
                        lcd.print("0");
                    lcd.print(mat_do);
                }
                if (step == 2)
                {
                    currentState = MAIN_MENU_1;
                    break;
                }
            }
            MAX_WISE = 4;
            MIN_WISE = 0;
        }
        void setup_time(bool status)
        {

            MAX_WISE = 2;
            MIN_WISE = 0;
            bool mode = false;
            unsigned long tick_press, tick_time_press, press_time = 10;
            clear_clow(0);
            int row = 0;
            lcd.clear();

            snprintf(time_task[0], sizeof(time_task[0]), "TIME OFF:  %02d:%02d", time_off.h, time_off.m);
            snprintf(time_task[1], sizeof(time_task[1]), "TIME ON :  %02d:%02d", time_on.h, time_on.m);

            volatile int last_count = 0;
            for (int i = 0; i < 4; ++i)
            {
                lcd.setCursor(1, i);
                lcd.print(time_task[i]);
            }
            while (status == true)
            {

                if (last_count != count)
                {
                    lcd.setCursor(0, last_count);
                    lcd.print(" ");
                    lcd.setCursor(0, count);
                    lcd.write(0);
                    lcd.setCursor(10, 3);
                    lcd.print(count);
                    last_count = count;
                }
                if (digitalRead(BUTTON) == 0 && mode == false)
                {
                    tick_press = millis();
                    mode = true;
                }
                if (mode == true)
                    tick_time_press = (uint32_t)millis();

                if (tick_time_press - tick_press > press_time && mode == true)
                {
                    if (digitalRead(BUTTON) == 0 && count == 0)
                    {
                        set_time_off(true);
                    }
                    if (digitalRead(BUTTON) == 0 && count == 1)
                    {
                        set_time_on(true);
                    }
                    if (digitalRead(BUTTON) == 0 && count == 2)
                    {
                        MAX_WISE = 4;
                        MIN_WISE = 0;
                        currentState = MAIN_MENU_1;
                        break;
                    }
                }
                if (((uint32_t)millis()) - tick_press > press_time && mode == true)
                {
                    mode = false;
                    tick_press = 0;
                }
            }
        }
        void hieu_chuan_can(bool status)
        {
            MAX_WISE = 2;
            MIN_WISE = 0;
            bool mode = false;
            unsigned long tick_press, tick_time_press, press_time = 15;
            clear_clow(0);
            int row = 0;
            lcd.clear();
            int last_count = 0;
            snprintf(load_cell_task[0], sizeof(load_cell_task[0]), "SO KG   :  %02u", so_can_cali);

            for (int i = 0; i < 4; ++i)
            {
                lcd.setCursor(1, i);
                lcd.print(load_cell_task[i]);
            }
            while (status == true)
            {
                lcd.setCursor(10, 3);
                lcd.print(count);
                if (last_count != count)
                {
                    lcd.setCursor(0, last_count);
                    lcd.print(" ");
                    lcd.setCursor(0, count);
                    lcd.write(0);
                    last_count = count;
                }

                if (digitalRead(BUTTON) == 0 && mode == false)
                {
                    tick_press = millis();
                    mode = true;
                }
                if (mode == true)
                    tick_time_press = (uint32_t)millis();

                if (tick_time_press - tick_press > press_time && mode == true)
                {
                    if (digitalRead(BUTTON) == 0 && count == 0)
                    {
                        MAX_WISE = 2;
                        MIN_WISE = 0;
                        currentState = NHAP_CAN_CALI;
                        break;
                    }
                    if (digitalRead(BUTTON) == 0 && count == 1)
                    {
                    }
                    if (digitalRead(BUTTON) == 0 && count == 2)
                    {
                        MAX_WISE = 4;
                        MIN_WISE = 0;
                        currentState = MAIN_MENU_2;
                        break;
                    }
                }
                if (((uint32_t)millis()) - tick_press > press_time && mode == true)
                {
                    mode = false;
                    tick_press = 0;
                }
            }
            count = 0;
        }
        void nhap_so_can(bool status)
        {
            bool mode = false;
            unsigned long tick_press, tick_time_press, press_time = 15;
            volatile int step = 0;
            volatile int last_step = 0;
            clear_clow(0);
            int row = 0;
            count = so_can_cali;
            while (status == true)
            {
                lcd.setCursor(10, 3);
                lcd.print(step);

                if (digitalRead(BUTTON) == 0 && mode == false)
                {
                    tick_press = millis();
                    mode = true;
                }
                if (mode == true)
                    tick_time_press = (uint32_t)millis();

                if (tick_time_press - tick_press > press_time && mode == true)
                {
                    while (digitalRead(BUTTON) == 0)
                    {
                    };
                    step = step + 1;
                    if (step >= 2)
                        step = 2;
                }
                if (((uint32_t)millis()) - tick_press > press_time && mode == true)
                {
                    mode = false;
                    tick_press = 0;
                }
                if (last_step != step)
                {
                    if (step == 1)
                    {
                        lcd.setCursor(10, row);
                        lcd.print("->");
                        last_step = step;
                    }
                    if (step == 2)
                    {
                        MAX_WISE = 2;
                        MIN_WISE = 0;
                        currentState = HIEU_CHAN_CAN;
                        delay(10);
                        break;
                    }
                }
                if (step == 1)
                {
                    MAX_WISE = 100;
                    MIN_WISE = 0;
                    so_can_cali = count;
                    if (so_can_cali > MAX_WISE)
                        so_can_cali = MAX_WISE;
                    lcd.setCursor(12, row);
                    if (so_can_cali < 10)
                        lcd.print("00");
                    if (so_can_cali > 10 && so_can_cali < 99)
                        lcd.print("0");
                    lcd.print(so_can_cali);
                }
                if (step == 2)
                {
                    MAX_WISE = 2;
                    MIN_WISE = 0;
                    currentState = HIEU_CHAN_CAN;
                    delay(10);
                    break;
                }
            }
        }
        void high_speed(bool status)
        {
            bool mode = false;
            unsigned long tick_press, tick_time_press, press_time = 15;
            volatile int step = 0;
            volatile int last_step = 0;

            clear_clow(0);
            int row = 0;
            count = van_toc_cao;

            while (status == true)
            {
                lcd.setCursor(10, 3);
                lcd.print(step);

                if (digitalRead(BUTTON) == 0 && mode == false)
                {
                    tick_press = millis();
                    mode = true;
                }
                if (mode == true)
                    tick_time_press = (uint32_t)millis();

                if (tick_time_press - tick_press > press_time && mode == true)
                {
                    while (digitalRead(BUTTON) == 0)
                    {
                        ;
                    }
                    step = step + 1;
                }
                if (((uint32_t)millis()) - tick_press > press_time && mode == true)
                {
                    mode = false;
                    tick_press = 0;
                }
                if (last_step != step)
                {
                    if (step == 1)
                    {
                        lcd.setCursor(10, row);
                        lcd.print("->");
                        last_step = step;
                        MAX_WISE = 100;
                        MIN_WISE = 0;
                    }
                    if (step == 2)
                    {
                        lcd.setCursor(10, row);
                        lcd.print("  ");
                    }
                }
                if (step == 1)
                {
                    van_toc_cao = count;
                    if (van_toc_cao > MAX_WISE)
                        van_toc_cao = MAX_WISE;
                    lcd.setCursor(12, row);
                    if (van_toc_cao < 10)
                        lcd.print("00");
                    if (van_toc_cao > 10 && van_toc_cao < 99)
                        lcd.print("0");
                    lcd.print(van_toc_cao);
                }

                if (step == 2)
                {
                    count = 0;
                    MAX_WISE = 2;
                    MIN_WISE = 0;
                    currentState = VAN_TOC;
                    break;
                }
            }
        }
        void low_speed(bool status)
        {
            bool mode = false;
            unsigned long tick_press, tick_time_press, press_time = 15;
            int step = 0;
            int last_step = 0;

            clear_clow(0);
            int row = 1;
            count = van_toc_thap;

            while (status == true)
            {
                lcd.setCursor(10, 3);
                lcd.print(step);

                if (digitalRead(BUTTON) == 0 && mode == false)
                {
                    tick_press = millis();
                    mode = true;
                }
                if (mode == true)
                    tick_time_press = (uint32_t)millis();

                if (tick_time_press - tick_press > press_time && mode == true)
                {
                    while (digitalRead(BUTTON) == 0)
                    {
                    };
                    step = step + 1;
                }
                if (((uint32_t)millis()) - tick_press > press_time && mode == true)
                {
                    mode = false;
                    tick_press = 0;
                }
                if (last_step != step)
                {
                    if (step == 1)
                    {
                        lcd.setCursor(10, row);
                        lcd.print("->");
                        last_step = step;
                        MAX_WISE = 100;
                        MIN_WISE = 0;
                    }
                    if (step == 2)
                    {
                        lcd.setCursor(10, row);
                        lcd.print("  ");
                    }
                }
                if (step == 1)
                {
                    van_toc_thap = count;
                    if (van_toc_thap > MAX_WISE)
                        van_toc_thap = MAX_WISE;
                    lcd.setCursor(12, row);
                    if (van_toc_thap < 10)
                        lcd.print("00");
                    if (van_toc_thap > 10 && van_toc_thap < 99)
                        lcd.print("0");
                    lcd.print(van_toc_thap);
                }

                if (step == 2)
                {

                    count = 1;
                    MAX_WISE = 2;
                    MIN_WISE = 0;
                    currentState = VAN_TOC;
                    break;
                }
            }
        }
        void cai_dat_van_toc(bool status)
        {

            MAX_WISE = 2;
            MIN_WISE = 0;
            unsigned long tick_press, tick_time_press, press_time = 15;
            clear_clow(0);
            int row = 0;
            lcd.clear();
            int last_count;

            snprintf(set_van_toc[0], sizeof(set_van_toc[0]), "HIGH    :  %03u", van_toc_cao);
            snprintf(set_van_toc[1], sizeof(set_van_toc[1]), "LOW     :  %03u", van_toc_thap);

            bool mode = false;
            for (int i = 0; i < 4; ++i)
            {
                lcd.setCursor(1, i);
                lcd.print(set_van_toc[i]);
            }
            while (status == true)
            {
                if (last_count != count)
                {
                    lcd.setCursor(0, last_count);
                    lcd.print(" ");
                    lcd.setCursor(0, count);
                    lcd.write(0);
                    last_count = count;
                }

                if (digitalRead(BUTTON) == 0 && mode == false)
                {
                    tick_press = millis();
                    mode = true;
                }
                if (mode == true)
                    tick_time_press = (uint32_t)millis();

                if (tick_time_press - tick_press > press_time && mode == true)
                {
                    if (digitalRead(BUTTON) == 0 && count == 0)
                    {
                        high_speed(true);
                    }
                    if (digitalRead(BUTTON) == 0 && count == 1)
                    {
                        low_speed(true);
                    }
                    if (digitalRead(BUTTON) == 0 && count == 2)
                    {
                        count = 0;
                        MAX_WISE = 4;
                        MIN_WISE = 0;
                        currentState = MAIN_MENU_2;
                        break;
                    }
                }
                if (((uint32_t)millis()) - tick_press > press_time && mode == true)
                {
                    mode = false;
                    tick_press = 0;
                }
            }
        }

        void menu_task_2()
        {
            for (int i = 0; i <= 3; ++i)
            {
                lcd.setCursor(1, i);
                lcd.print(menuItems_2[i]);
            }
        }
        void menu_task_1()
        {
            lcd.clear();

            snprintf(menuItems_1[0], sizeof(menuItems_1[0]), "MAT DO:      %06d", mat_do);
            for (int i = 0; i <= 3; ++i)
            {
                lcd.setCursor(1, i);
                lcd.print(menuItems_1[i]);
            }
        }
        void display_task_2(bool status)
        {
            MAX_WISE = 4;
            MIN_WISE = 0;
            bool mode = false;
            unsigned long tick_press, tick_time_press, press_time = 15;
            int last_count;
            menu_task_2();
            while (status == true)
            {

                if (last_count != count && last_count != 4)
                {
                    lcd.setCursor(0, last_count);
                    lcd.print(" ");
                    last_count = count;
                }
                if (last_count == 4 && last_count != count)
                {
                    lcd.setCursor(19, 3);
                    lcd.print(" ");
                    last_count = count;
                }
                if (count == 4)
                {
                    lcd.setCursor(19, 3);
                    lcd.write(0);
                    lcd.setCursor(10, 3);
                    lcd.print(count);
                }
                if (count <= 3)
                {
                    lcd.setCursor(0, count);
                    lcd.write(0);
                    lcd.setCursor(10, 3);
                    lcd.print(count);
                }
                if (digitalRead(BUTTON) == 0 && mode == false)
                {
                    tick_press = millis();
                    mode = true;
                }
                if (mode == true)
                    tick_time_press = (uint32_t)millis();

                if (tick_time_press - tick_press > press_time && mode == true)
                {
                    if (digitalRead(BUTTON) == 0 && count == 0)
                    {

                        currentState = VAN_TOC;
                        break;
                    }
                    if (digitalRead(BUTTON) == 0 && count == 1)
                    {

                        currentState = HIEU_CHAN_CAN;
                        break;
                    }
                    if (digitalRead(BUTTON) == 0 && count == 2)
                    {
                        currentState = MAIN_MENU_2;
                        break;
                    }
                    if (digitalRead(BUTTON) == 0 && count == 3)
                    {

                        currentState = MAIN_MENU_1;
                        break;
                    }
                    if (digitalRead(BUTTON) == 0 && count == 4)
                    {

                        currentState = EXIT;
                        break;
                    }
                }
                if (((uint32_t)millis()) - tick_press > press_time && mode == true)
                {
                    mode = false;
                    tick_press = 0;
                }
            }
        }

        void mode_setup(bool status)
        {

            MAX_WISE = 4;
            MIN_WISE = 0;
            unsigned long tick_press, tick_time_press, press_time = 15;
            menu_task_1();
            volatile int last_count = 0;
            bool mode = false;

            while (status)
            {

                if (count != last_count && last_count != 4)
                {
                    lcd.setCursor(0, last_count);
                    lcd.print(" ");
                    last_count = count;
                }
                if (last_count == 4 && count != last_count)
                {
                    lcd.setCursor(19, 3);
                    lcd.print(" ");
                    last_count = count;
                }
                if (count == 4)
                {
                    lcd.setCursor(19, 3);
                    lcd.write(0);
                }
                if (count <= 3)
                {
                    lcd.setCursor(0, count);
                    lcd.write(0);
                }
                if (digitalRead(BUTTON) == 0 && mode == false)
                {
                    tick_press = millis();
                    mode = true;
                }
                if (mode == true)
                    tick_time_press = (uint32_t)millis();

                if (tick_time_press - tick_press > press_time && mode == true)
                {
                    if (digitalRead(BUTTON) == 0 && count == 0)
                    {
                        currentState = MAT_DO;
                        break;
                    }
                    if (digitalRead(BUTTON) == 0 && count == 1)
                    {
                        currentState = THOI_GIAN_PHUN;
                        break;
                    }
                    if (digitalRead(BUTTON) == 0 && count == 2)
                    {

                        clear_clow(0);
                        currentState = THOIGIANPHUNNGHI;
                        break;
                    }
                    if (digitalRead(BUTTON) == 0 && count == 3)
                    {
                        clear_clow(0);
                        save_data::saveData();
                        currentState = EXIT;
                        break;
                    }
                    if (digitalRead(BUTTON) == 0 && count == 4)
                    {
                        mode = false;
                        tick_press = 0;
                        currentState = MAIN_MENU_2;
                        break;
                    }
                }
                if (((uint32_t)millis()) - tick_press > press_time && mode == true)
                {
                    mode = false;
                    tick_press = 0;
                    currentState = MAIN_MENU_1;
                    break;
                }
            }
        }
        void setting_state()
        {

            currentState = MAIN_MENU_1;
            display::count = 0;
            display::countLast = -1;
            display::state_mode = true;
            while (state_mode)
            {
                switch (currentState)
                {
                case MAIN_MENU_1:
                    mode_setup(true);
                    break;
                case MAIN_MENU_2:
                    display_task_2(true);
                    break;
                case MAT_DO:
                    set_mat_do(true);
                    break;
                case THOI_GIAN_PHUN:
                    setup_time(true);
                    break;
                case THOIGIANPHUNNGHI:
                    cau_hinh_thoi_gian(true);
                    save_data::saveData();

                    break;
                case VAN_TOC:
                    cai_dat_van_toc(true);
                    break;
                case HIEU_CHAN_CAN:
                    hieu_chuan_can(true);
                    break;
                case NHAP_CAN_CALI:
                    nhap_so_can(true);
                    break;
                case EXIT:
                    save_data::saveData();
                    wifi::sendata_mqtt(0);
                    state_mode = false;
                    break;
                }
            }
            detachInterrupt(ENCODER_A);
            detachInterrupt(ENCODER_B);
        }
    }
}

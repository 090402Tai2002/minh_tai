#include "lib.h"
namespace main
{

    namespace escct
    {
        enum state
        {
            STOP,
            RUN
        };
        state esc_state = STOP;
        TimerHandle_t timer_start_motor, timer_stop_motor, timer_dc_xa;
        Servo esc; // Khai báo đối tượng ESC
        int freq = 1000;
        int escPin = 13; // Chân điều khiển ESC (có thể thay đổi

        // PID constants (điều chỉnh để giảm dao động)
        float kp_1 = 4.0;
        float kd_1 = 0;
        float ki_1 = 6.0;

        // Variables for timing
        long prevT = 0;
        float eprev_1 = 0;
        uint8_t soft_count = 0;
        float eintegral_1 = 0;

        void control_speed(uint16_t speed);

        void tat_dc_xa(TimerHandle_t xTimer)
        {
            int hours = display::off_time_seconds[display::cycle_off] / 3600;          // Tính số giờ
            int minutes = (display::off_time_seconds[display::cycle_off] % 3600) / 60; // Tính số phút
            int seconds = display::off_time_seconds[display::cycle_off] % 60;
            time_set::cacula_time_on(hours, minutes, seconds);
            main::action::action_state_auto =  main::action::STOP;
            control_speed(0);
        }

        void tat_motor_thuc_an(TimerHandle_t xTimer)
        {
            digitalWrite(DC_XA, 0);
            timer_dc_xa = xTimerCreate("OneShot", 5000 / portTICK_PERIOD_MS, pdFALSE, 0, tat_dc_xa);
            xTimerStart(timer_dc_xa, 0);
        }

        void chay_motor_thuc_an(TimerHandle_t xTimer)
        {
            digitalWrite(DC_XA, 1);
            timer_stop_motor = xTimerCreate("OneShot", display::on_time_seconds[display::cycle_on] * 1000 / portTICK_PERIOD_MS, pdFALSE, 0, tat_motor_thuc_an);
            Serial.print(" thoi gian chay: ");
            Serial.println(display::on_time_seconds[display::cycle_on]);
            xTimerStart(timer_stop_motor, 0);
        }

        void bat_dau_chu_ky_cho_an()
        {
            control_speed(60);
            timer_start_motor = xTimerCreate("OneShot", 5000 / portTICK_PERIOD_MS, pdFALSE, 0, chay_motor_thuc_an);
            xTimerStart(timer_start_motor, 0);
        }

        void tat_tat_ca_thiet_bi()
        {
            if (timer_dc_xa != NULL)
            {
                xTimerStop(timer_dc_xa, 0);   // Dừng Timer nếu nó đang chạy
                xTimerDelete(timer_dc_xa, 0); // Xóa Timer
                timer_dc_xa = NULL;           // Đặt lại về NULL
            }
            if (timer_start_motor != NULL)
            {
                xTimerStop(timer_start_motor, 0);   // Dừng Timer nếu nó đang chạy
                xTimerDelete(timer_start_motor, 0); // Xóa Timer
                timer_start_motor = NULL;           // Đặt lại về NULL
            }
            if (timer_stop_motor != NULL)
            {
                xTimerStop(timer_stop_motor, 0);   // Dừng Timer nếu nó đang chạy
                xTimerDelete(timer_stop_motor, 0); // Xóa Timer
                timer_stop_motor = NULL;           // Đặt lại về NULL
            }
            digitalWrite(DC_XA, 0);
            control_speed(0);
        }

        void setup()
        {
            ESP32PWM::allocateTimer(0);
            ESP32PWM::allocateTimer(1);
            ESP32PWM::allocateTimer(2);
            ESP32PWM::allocateTimer(3);
            esc.setPeriodHertz(50);
            pinMode(DC_XA, OUTPUT);

            esc.attach(escPin, 1000, 2000);
            esc.writeMicroseconds(2000);
            Serial.println("SETTING ESC");
            delay(5000);
            esc.writeMicroseconds(1000);
            delay(6000);
            Serial.println("SETTING ESC DONE");
        }

        void control_speed(uint16_t speed)
        {
            speed = map(speed, 0, 100, 1000, 2000);
            esc.writeMicroseconds(speed); // Gửi xung PPM
        }

        void pid_control(float so_ky_cho_an)
        {
            long currT = micros();
            float deltaT = ((float)(currT - prevT)) / 1.0e6;
            float so_can = can::scale.get_units(2);
            Serial.print("one reading:\t");
            Serial.println(can::scale.get_units(2), 1);
            float taget = so_can - so_ky_cho_an;
            if (taget <= 0)
                taget = 0;
            prevT = currT;
            // PID algorithm - Gain, integral, D
            float e_1 = so_can - taget; // sai
            const float alpha = 0.1;    // Bộ lọc tín hiệu lỗi
            float e_filtered_1 = alpha * e_1 + (1.0 - alpha) * eprev_1;
            eprev_1 = e_filtered_1;
            float dedt_1 = (e_1 - eprev_1) / deltaT; // D
            eintegral_1 += e_1 * deltaT;             // I
            float u_1 = kp_1 * e_1 + kd_1 * dedt_1 + ki_1 * eintegral_1;
            int pwr_1 = (int)fabs(u_1);
            pwr_1 = constrain(pwr_1, 255, 0); // PWM: 0 - 255
            analogWrite(DC_XA, pwr_1);
            Serial.println(pwr_1);
            eprev_1 = e_1;
        }

    }
}
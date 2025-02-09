#include "lib.h"
namespace main
{

    namespace can
    {
        HX711 scale;
        const int LOADCELL_DOUT_PIN = 4;
        const int LOADCELL_SCK_PIN = 5;
        
        void setup()
        {

            scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

          

            scale.tare(); // reset the scale to 0
            scale.set_scale(41.0798);
            // scale.set_scale(-471.497);                      // this value is obtained by calibrating the scale with known weights; see the README for details

            // Serial.println("After setting up the scale:");

            Serial.print("read: \t\t");
            Serial.println(scale.read()); // print a raw reading from the ADC

            // Serial.print("read average: \t\t");
            // Serial.println(scale.read_average(20)); // print the average of 20 readings from the ADC

            // Serial.print("get value: \t\t");
            // Serial.println(scale.get_value(5)); // print the average of 5 readings from the ADC minus the tare weight, set with tare()

            // Serial.print("get units: \t\t");
            // Serial.println(scale.get_units(5), 1); // print the average of 5 readings from the ADC minus tare weight, divided
            //                                        // by the SCALE parameter set with set_scale
            // Serial.println("Readings:");
        }
        
        void calibration()
        {
            if (scale.is_ready())
            {
                if (scale.is_ready())
                {
                    scale.set_scale();
                    delay(5000);
                    scale.tare();
                    delay(5000);
                    long reading = scale.get_units(10);
                }
                else
                {
                    Serial.println("HX711 not found.");
                }
                delay(1000);
            }
        }
        void read()
        {
            display::display_weigh(scale.get_units(5));
        }

    }

}
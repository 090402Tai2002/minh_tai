#include "lib.h"

namespace main
{
    namespace wifi
    {
        Preferences save;

        const char *mqtt_server = "192.168.1.89";

        const int mqtt_port = 1883;
        const char *mqtt_topic_sub = "shrimp/feeding_data";
        const char *mqtt_topic_sub_data = "device/config/get";
        const char *mqtt_topic_pub = "test_response";
        const char *mqtt_topic_pub_data = "device/config/put";
        const char *ssid = NULL;
        const char *password = NULL;

        WiFiClient espClient;
        PubSubClient mqttClient(espClient);
        void subdata_mqtt();
        void subdata_mqtt()
        {
            mqttClient.subscribe(mqtt_topic_sub);
            mqttClient.subscribe(mqtt_topic_sub_data);
            mqttClient.subscribe("mat_do");
            mqttClient.subscribe("on_time");
            mqttClient.subscribe("off_time");
            mqttClient.subscribe("thoi_gian_phun");
            mqttClient.subscribe("thoi_gian_nghi");
            mqttClient.subscribe("so_can_cali");
            mqttClient.subscribe("van_toc_cao");
            mqttClient.subscribe("van_toc_thap");
        }
        void save_wifi(const String &ssid, const String &pass)
        {
            if (!save.begin("wifi_config", false))
            {
                Serial.println("Không thể mở Preferences cho wifi_config");
                return;
            }

            save.putString("ssid", ssid);
            save.putString("password", pass);
            save.end();

            Serial.println("Thông tin Wi-Fi đã được lưu:");
            Serial.print("SSID: ");
            Serial.println(ssid);
        }

        bool setup_wifi()
        {
            if (!save.begin("wifi_config", false))
            {
                Serial.println("Không thể mở Preferences cho wifi_config");
                return false;
            }

            String saved_ssid = save.getString("ssid", "");
            String saved_password = save.getString("password", "");
            save.end();

            if (saved_ssid != "" && saved_password != "")
            {
                ssid = strdup(saved_ssid.c_str());
                password = strdup(saved_password.c_str());
                return true;
            }
            Serial.println("Không tìm thấy thông tin Wi-Fi trong bộ nhớ.");
            return false;
        }

        void connectWiFi()
        {
            Serial.println("Đang kết nối Wi-Fi...");
            WiFi.begin(ssid, password);
            int retry_count = 0;
            while (WiFi.status() != WL_CONNECTED && retry_count < 15)
            {
                Serial.print(".");
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                retry_count++;
            }

            if (WiFi.status() == WL_CONNECTED)
            {
                Serial.println("\nĐã kết nối Wi-Fi!");
                Serial.print("IP Address: ");
                Serial.println(WiFi.localIP());
            }
            else
            {
                Serial.println("\nKhông thể kết nối Wi-Fi sau 5 lần thử. Bật SmartConfig...");
                WiFi.mode(WIFI_AP_STA); // Đảm bảo chế độ AP và STA được bật
                WiFi.beginSmartConfig();

                // Đợi SmartConfig được thực hiện
                while (!WiFi.smartConfigDone())
                {
                    Serial.print(".");
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                }
                Serial.println("\nSmartConfig nhận được.");

                // Đợi Wi-Fi kết nối thành công
                while (WiFi.status() != WL_CONNECTED)
                {
                    Serial.print(".");
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                }
                Serial.println("\nWi-Fi đã kết nối thông qua SmartConfig.");
                Serial.print("IP Address: ");
                Serial.println(WiFi.localIP());

                // Lưu thông tin Wi-Fi mới
                save_wifi(WiFi.SSID(), WiFi.psk());
            }
        }
        // void sendata_mqtt(int value)
        // {
        //     if (value == 0)
        //     {
        //         // Create JSON object
        //         StaticJsonDocument<256> doc;
        //         doc["mat_do"] = display::mat_do;
        //         doc["gio_bat"] = display::time_on.h;
        //         doc["phut_bat"] = display::time_on.m;
        //         doc["gio_tat"] = display::time_off.h;
        //         doc["phut_tat"] = display::time_off.m;
        //         doc["thoi_gian_phun"] = display::cycle_on;
        //         doc["thoi_gian_nghi"] = display::cycle_off;
        //         doc["so_can_cali"] = display::so_can_cali;
        //         doc["van_toc_cao"] = display::van_toc_cao;
        //         doc["van_toc_thap"] = display::van_toc_thap;

        //         // Serialize JSON to string
        //         char jsonBuffer[256];
        //         serializeJson(doc, jsonBuffer);
        //         // Publish JSON string to MQTT topic
        //         mqttClient.publish(mqtt_topic_pub_data, jsonBuffer);
        //         Serial.println("JSON sent: ");
        //         Serial.println(jsonBuffer);
        //     }
        // }
        void publishDataTask(void *parameter)
        {
            while (true)
            {
                // Create JSON object
                StaticJsonDocument<256> doc;

                // doc["so_lan_phun"] = display::so_lan_phun;
                doc["mat_do"] = display::mat_do;
                doc["gio_bat"] = display::time_on.h;
                doc["phut_bat"] = display::time_on.m;
                doc["gio_tat"] = display::time_off.h;
                doc["phut_tat"] = display::time_off.m;
                doc["thoi_gian_phun"] = display::on_time_seconds[display::cycle_on];
                doc["thoi_gian_nghi"] = display::off_time_seconds[display::cycle_off];
                doc["so_can_cali"] = display::so_can_cali;
                doc["van_toc_cao"] = display::van_toc_cao;
                doc["van_toc_thap"] = display::van_toc_thap;
                // Serialize JSON to string
                char jsonBuffer[256];
                serializeJson(doc, jsonBuffer);
                // Publish JSON string to MQTT topic
                mqttClient.publish(mqtt_topic_pub_data, jsonBuffer);
                Serial.println("JSON sent: ");
                Serial.println(jsonBuffer);
                // Wait for 5 seconds
                vTaskDelay(5000 / portTICK_PERIOD_MS);
            }
        }
        void mqttCallback(char *topic, byte *message, unsigned int length)
        {

            Serial.print("Message arrived on topic: ");
            Serial.println(topic);
            if (String(topic) == mqtt_topic_sub)
            {
                // Convert payload to String
                String jsonPayload;
                for (unsigned int i = 0; i < length; i++)
                {
                    jsonPayload += (char)message[i];
                }
                Serial.println("Received JSON payload: " + jsonPayload);

                // Parse JSON
                StaticJsonDocument<256> doc;
                DeserializationError error = deserializeJson(doc, jsonPayload);

                if (error)
                {
                    Serial.print("JSON deserialization failed: ");
                    Serial.println(error.f_str());
                    return;
                }
                // Extract values from JSON
                int avg_length_mm = doc["avg_length_mm"];
                float weight = doc["weight"];
                float feed_percentage = doc["feed_percentage"];
                float feed_amount = doc["feed_amount"];

                // Print extracted values
                Serial.println("Average Length (mm): " + String(avg_length_mm));
                Serial.println("Weight: " + String(weight));
                Serial.println("Feed Percentage: " + String(feed_percentage));
                Serial.println("Feed Amount: " + String(feed_amount));

                // Publish response
                mqttClient.publish(mqtt_topic_pub, "feeding_data");
                jsonPayload = "";
            }

            if (String(topic) == "mat_do")
            {
                String jsonPayload;
                for (unsigned int i = 0; i < length; i++)
                {
                    jsonPayload += (char)message[i];
                }
                Serial.println("Received JSON payload: " + jsonPayload);

                // Parse JSON
                StaticJsonDocument<256> doc;
                DeserializationError error = deserializeJson(doc, jsonPayload);

                // Kiểm tra lỗi parse
                if (error)
                {
                    Serial.println("Error parsing JSON: " + String(error.f_str()));
                }
                else
                {
                    // Lấy giá trị mat_do từ JSON, giá trị này có thể là số nguyên
                    int mat_do_value = doc["mat_do"];                               // Đây là nơi lấy giá trị số nguyên từ "mat_do"
                    display::mat_do = mat_do_value;                                 // Gán giá trị cho display::mat_do
                    Serial.println("MẬT ĐỘ NHẬN ĐƯỢC: " + String(display::mat_do)); // In ra giá trị nhận được
                }
                // Giải phóng jsonPayload nếu không cần thiết nữa
                jsonPayload = "";
            }
            if (String(topic) == "on_time") // Nhận dữ liệu thời gian cho phép chạy
            {
                String jsonPayload;
                for (unsigned int i = 0; i < length; i++)
                {
                    jsonPayload += (char)message[i];
                }
                Serial.println("Received JSON payload: " + jsonPayload);

                // Parse JSON
                StaticJsonDocument<256> doc;
                DeserializationError error = deserializeJson(doc, jsonPayload);

                // Kiểm tra lỗi parse
                if (error)
                {
                    Serial.println("Error parsing JSON: " + String(error.f_str()));
                }
                else
                {
                    // Trích xuất giá trị từ JSON
                    int hour = doc["hour"];             // Lấy giá trị hour
                    int minute = doc["minute"];         // Lấy giá trị minute
                    const char *status = doc["status"]; // Lấy giá trị status (chuỗi)
                    display::time_on.h = hour;
                    display::time_on.m = minute;
                    // In các giá trị ra Serial Monitor
                    Serial.print("Hour: ");
                    Serial.println(display::time_on.h);
                    Serial.print("Minute: ");
                    Serial.println(display::time_on.m);
                    Serial.print("Status: ");
                    Serial.println(status);
                }
                // Giải phóng jsonPayload nếu không cần thiết nữa
                jsonPayload = "";
            }
            if (String(topic) == "off_time") // Nhận dữ liệu thời gian tắt của thiết bị
            {
                String jsonPayload;
                for (unsigned int i = 0; i < length; i++)
                {
                    jsonPayload += (char)message[i];
                }
                Serial.println("Received JSON payload: " + jsonPayload);

                // Parse JSON
                StaticJsonDocument<256> doc;
                DeserializationError error = deserializeJson(doc, jsonPayload);

                // Kiểm tra lỗi parse
                if (error)
                {
                    Serial.println("Error parsing JSON: " + String(error.f_str()));
                }
                else
                {
                    // Trích xuất giá trị từ JSON
                    int hour = doc["hour"];             // Lấy giá trị hour
                    int minute = doc["minute"];         // Lấy giá trị minute
                    const char *status = doc["status"]; // Lấy giá trị status (chuỗi)
                    display::time_off.h = hour;
                    display::time_off.m = minute;
                    // In các giá trị ra Serial Monitor
                    Serial.print("Hour: ");
                    Serial.println(display::time_off.h);
                    Serial.print("Minute: ");
                    Serial.println(display::time_off.m);
                    Serial.print("Status: ");
                    Serial.println(status);
                }
                // Giải phóng jsonPayload nếu không cần thiết nữa
                jsonPayload = "";
            }

        } // cần nhận dữ liệu của mqtt và đồng bộ dữ liệu
        void connectMQTT()
        {
            while (!mqttClient.connected())
            {
                Serial.println("Kết nối MQTT broker...");
                if (mqttClient.connect("ESP32Client"))
                {
                    Serial.println("Đã kết nối!");
                    subdata_mqtt();
                }
                else
                {
                    Serial.print("Kết nối thất bại, trạng thái: ");
                    Serial.println(mqttClient.state());
                    vTaskDelay(2000 / portTICK_PERIOD_MS);
                }
            }
        }

        void mqttTask(void *param)
        {
            for (;;)
            {
                if (!mqttClient.connected())
                {
                    connectMQTT();
                }
                mqttClient.loop();
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
        }

        void main_setup()
        {
            Serial.begin(115200);

            if (setup_wifi())
            {
                connectWiFi();
            }
            else
            {
                Serial.println("Không có thông tin Wi-Fi. Bật SmartConfig...");
                WiFi.mode(WIFI_AP_STA);
                WiFi.beginSmartConfig();

                while (!WiFi.smartConfigDone())
                {
                    Serial.print(".");
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                }
                Serial.println("\nSmartConfig nhận được.");

                while (WiFi.status() != WL_CONNECTED)
                {
                    Serial.print(".");
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                }
                save_wifi(WiFi.SSID(), WiFi.psk());
                Serial.println("\nWi-Fi đã kết nối và thông tin đã được lưu.");
            }
            mqttClient.setServer(mqtt_server, mqtt_port);
            mqttClient.setCallback(mqttCallback);
            xTaskCreatePinnedToCore(mqttTask, "MQTT Task", 4096, NULL, 1, NULL, 1);
            xTaskCreate(publishDataTask, "PublishData", 2048, NULL, 1, NULL);
        }
    }
}

#include "WiFi.h"

void setup() 
{

  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();


  Serial.println("Waiting for SmartConfig.");
  while (!WiFi.smartConfigDone()) {
    delay(500);
    Serial.print(".");
  }
    Serial.println("");
    Serial.println("SmartConfig received.");


  Serial.println("Waiting for WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi Connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

}
void loop() 
{

}

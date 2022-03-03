#include <WiFi.h>

const char* ssid     = "I love my job";
const char* password = "Skribbl.io";

void setup()
{
    Serial.begin(9600);
    Serial.println("\n[*] Creating AP ...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.print("[+] AP Created with IP Gateway ");
    Serial.println(WiFi.softAPIP());
}

void loop(){

}

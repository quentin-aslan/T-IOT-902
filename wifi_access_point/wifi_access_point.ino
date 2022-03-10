#include <WiFi.h>

// CREDENTIAL WIFI !! (DONT FORGET)
const char* ssid = "Mi 11 Lite";
const char* password = "quentin1";

// CREDENTIAL AP
const char* ssid_AP     = "I love my job";
const char* password_AP = "Skribbl.io";

void setup() {
    Serial.begin(9600);

    // CONNECTION AU REASEAU WIFI
    WiFi.begin(ssid, password);
    Serial.println("Connecting to the WIFI ...");

    // Si au bout de 20 secondes la connexion au WIFI échoue, on créer l'access point
    int timer = 0;
    while ((WiFi.status() != WL_CONNECTED) && timer != 20) {
        delay(500);
        Serial.print(".");
        timer = timer + 1;
    }

    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.print("Connected to WiFi network with IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
      Serial.println("Connection à la wifi echoué, creation d'un access point");
        startWifiAccessPoint();
    }
}

void startWifiAccessPoint() {
    Serial.println("\n[*] Creating AP ...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.print("[+] AP Created with IP Gateway ");
    Serial.println(WiFi.softAPIP());
}

void loop(){

}

#include <WiFi.h>
#include <HTTPClient.h>

// CREDENTIAL WIFI !! (DONT FORGET)
const char* ssid = "Mi 11 Lite";
const char* password = "quentin1";

// ENDPOINTS
const char* endpoint = "http://89.82.200.242:31830/test";
int timerDelay = 10000; // Toutes les 10 secondes on rentre dans la boucle

void setup() {
    Serial.begin(9600);

    // CONNECTION AU REASEAU WIFI
    WiFi.begin(ssid, password);
    Serial.println("Connecting");

    // Tant que l'ESP n'est pas connecté au WIFI, on boucle, toutes les 500ms ...
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("On est connecté à la wifi");

        // Avec des variables =>
        // char datas[150]
        // Exemple : sprintf(datas, "{\"humidity\":\"%f\",\"temperature\":\"%f\",\"realFeel\":\"%f\",\"location\":\"%s\"}", humidityFloat, temperatureFloat, realFeelFloat, DHT22_LOCATION);
        sendHTTPRequestPOST("\"msg\": \"I love my job\"");
    } else {
        Serial.println("WiFi Disconnected");
    }
    delay(timerDelay);
}

// Envoie une requête HTTP.
void sendHTTPRequestPOST(String payload) {
    HTTPClient http;
    http.begin(endpoint);
    http.addHeader("Content-Type", "application/json");
    int datasStatus = http.POST(payload);
    Serial.print("HTTP Response code : ");
    Serial.println(datasStatus);
    http.end();
}

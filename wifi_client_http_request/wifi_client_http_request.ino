#include <WiFi.h>
#include <HTTPClient.h>

// CREDENTIAL WIFI !! (DONT FORGET)
const char* ssid = "";
const char* password = "";

// ENDPOINTS
const char* endpoint = "http://192.168.1.50:31830/dht22";

void setup() {
    Serial.begin(115200);

    // CONNECTION AU REASEAU WIFI
    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED) { // ON NE CONTINUE PAS TANT QUE L'ESP EST PAS CONNECTÉ A LA WIFI, on essaye toutes les 500ms
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
        sendHTTPRequestPOST("I love my job");
    } else {
        Serial.println("WiFi Disconnected");
    }
    delay(timerDelay);
}

void sendHTTPRequestPOST(payload) {
    HTTPClient http;

    // ALL
    char datas[150];
    sprintf(datas, payload);
    // Exemple : sprintf(datas, "{\"humidity\":\"%f\",\"temperature\":\"%f\",\"realFeel\":\"%f\",\"location\":\"%s\"}", humidityFloat, temperatureFloat, realFeelFloat, DHT22_LOCATION);

    http.begin(endpoint);
    http.addHeader("Content-Type", "application/json");
    int datasStatus = http.POST(datas);
    Serial.print("HTTP Response code : ");
    Serial.println(datasStatus);
    http.end();
}
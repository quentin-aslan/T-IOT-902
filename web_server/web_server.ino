#include <WiFi.h>
#include <WebServer.h> // https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer

/**
 * DECLARE VARIABLE FOR ALL PROGRAM
 */

/**
 * WIFI CLIENT & ACCESS POINT
 */

// WIFI CLIENT
const char* ssid = "Mi 11 Lite";
const char* password = "quentin1";

// WIFI ACCESS POINT
const char* AP_ssid     = "I love my job";
const char* AP_password = "Skribbl.io";
IPAddress AP_local_ip(192,168,3,1);
IPAddress AP_gateway(192,168,3,1);
IPAddress AP_subnet(255,255,255,0);
bool WIFI_isStarted = false;

// Si au bout de 20 secondes la connexion au wifi échoue, on créer l'access point
void tryConnectWifiClient() {
    WiFi.begin(ssid, password);
    Serial.println("Connecting to the WIFI ...");

    int timer = 0;
    while ((WiFi.status() != WL_CONNECTED) && timer != 20) {
        delay(500);
        Serial.print(".");
        timer = timer + 1;
    }

    if(WiFi.status() == WL_CONNECTED) { // Connection au wifi réussie
        Serial.println("");
        Serial.print("Connected to WiFi network with IP Address: ");
        Serial.println(WiFi.localIP());
        WIFI_isStarted = true;
    } else { // Connection au wifi échoué, création de l'access point
        Serial.println("Connection à la wifi echoué, creation d'un access point");
        startWifiAccessPoint();
    }
}

void startWifiAccessPoint() {
    Serial.println("\n[*] Creating AP ...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_ssid, AP_password);
    WiFi.softAPConfig(AP_local_ip, AP_gateway, AP_subnet); // Information about access point AP_subnet
    Serial.print("[+] AP Created with IP AP_gateway ");
    Serial.println(WiFi.softAPIP());
    delay(100);
    WIFI_isStarted = true;
}

/**
 * WEB SERVER
 * https://lastminuteengineers.com/creating-esp32-web-server-arduino-ide/
 */
int WebServer_PORT = 80;
WebServer WebServer_SERVER(WebServer_PORT);
bool WebServer_isStarted = false;
String header;

void initWebServer() {
    WebServer_SERVER.on("/", WebServer_OnConnect);
    WebServer_SERVER.on("/updateDatas", WebServer_UpdateDatas);
    WebServer_SERVER.onNotFound(WebServer_404);
    WebServer_SERVER.begin();
    Serial.println("HTTP server started on port :");
    Serial.print(WebServer_PORT);
    WebServer_isStarted = true;
}

void WebServer_OnConnect() {
    Serial.println("Client connect to : /");
    WebServer_SERVER.send(200, "text/html", SendHTML());
}

void WebServer_UpdateDatas() {
    Serial.println("Client connect to : /");

    if(WebServer_SERVER.args() == 0) { // Si il y a aucun arguments, on renvoie la page d'accueil !
        WebServer_OnConnect();
    } else {
        if(server.hasArg("gateway")) { // SI C'EST LA GATEWAY

        } else if (server.hasArg("sensor")) { // SI C'EST UN SENSOR
            if(server.hasArg("LoRa_address") && server.hasArg("name") && server.hasArgs()) {

            }
        }
    }

    WebServer_SERVER.send(200, "text/html", SendHTML());
}

void WebServer_404(){
    WebServer_SERVER.send(404, "text/plain", "Not found");
}

String SendHTML(){
    String ptr = "<!DOCTYPE html> <html>\n";
    ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    ptr +="<title>IOT GROUP 19</title>\n";
    ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
    ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
    ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
    ptr +=".button-on {background-color: #3498db;}\n";
    ptr +=".button-on:active {background-color: #2980b9;}\n";
    ptr +=".button-off {background-color: #34495e;}\n";
    ptr +=".button-off:active {background-color: #2c3e50;}\n";
    ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
    ptr +="</style>\n";
    ptr +="</head>\n";
    ptr +="<body>\n";
    ptr +="<h1>ESP32 Web Server</h1>\n";
    ptr +="<h3>Using Access Point(AP) Mode</h3>\n";

    ptr +="</body>\n";
    ptr +="</html>\n";
    return ptr;
}

void setup() {
    Serial.begin(9600);

    /**
     * Connection au WIFI CLIENT.
     * Si au bout de 20 secondes la connexion au wifi échoue, on créer l'access point
     */
     tryConnectWifiClient();
}

void loop(){
  if(WIFI_isStarted && !WebServer_isStarted) {
    initWebServer();
  }
  
  if(WebServer_isStarted) {
    WebServer_SERVER.handleClient();
  }

}

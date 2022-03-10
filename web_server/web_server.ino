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
const char* AP_password = "quentin1";
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
    Serial.println(WebServer_SERVER.args());

    if(WebServer_SERVER.args() == 0) { // Si il y a aucun arguments, on renvoie la page d'accueil !
        WebServer_OnConnect();
    } else {
        if(WebServer_SERVER.hasArg("lora_address") && WebServer_SERVER.hasArg("name")) {
            String lora_address = WebServer_SERVER.arg("lora_address");
            String name = WebServer_SERVER.arg("name");
            Serial.println("name");
            Serial.println(name);
            Serial.println("lora_address");
            Serial.println(lora_address);

            if(WebServer_SERVER.hasArg("lora_gateway")) {
                String lora_gateway = WebServer_SERVER.arg("lora_gateway");
                Serial.println("C'est le capteur.");
            } else {
                Serial.println("C'est une gateway.");
                // ICI IL FAUT RENTRER LES INFOS DU SENSOR COMMUNITY PROJECT !!!
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

    // Fonction qui hide/show le LoraGateway
    ptr +="<script>";
    ptr +="function isGateway() {";
    ptr +="if(document.getElementById('isGateway').checked){ alert('a'); document.getElementById('p_lora_gateway').style.display = 'none';}";
    ptr +="if(!document.getElementById('isGateway').checked){ alert('b'); document.getElementById('p_lora_gateway').style.display = 'block';}";
    ptr +="}";
    ptr +="</script>";

    ptr +="<h1>IOT WEB SERVIER</h1>\n";
    ptr +="<h3>CONFIGURATION MODE</h3>\n";
    ptr +="<h4>CONFIGURATION MODE</h4>\n";
    ptr +="<form action='/updateDatas' method='get'>";

    // Name
    ptr +="<p>\n";
    ptr +="<label for='name'>Name</label>\n";
    ptr +="<input name='name' type='text' name='Lora Gateway / Sensor xxx'>\n";
    ptr +="</p>\n";


    // Gateway checkbox
    ptr +="<p>\n";
    ptr +="<label for='isGateway'>Gateway</label>\n";
    ptr +="<input name='isGateway' id='isGateway' type='checkbox' onclick='isGateway()'>\n";
    ptr +="</p>\n";

    // LoRa Address
    ptr +="<p>\n";
    ptr +="<label for='lora_address'>LoRa Address</label>\n";
    ptr +="<input name='lora_address' type='text'>\n";
    ptr +="</p>\n";

    // LoRa Gateway (FOR SENSOR ONLY)
    ptr +="<p id='p_lora_gateway' style='display: none'>\n";
    ptr +="<label for='lora_gateway'>LoRa Gateway</label>\n";
    ptr +="<input name='lora_gateway' type='text'>\n";
    ptr +="</p>\n";

    ptr +="<p>\n";
    ptr +="<input type='submit'>\n";
    ptr +="</p>\n";
    ptr +="</form>\n";

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

/*#include <M5Stack.h>*/
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h> // https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer
#include <string.h>

/**
 * DECLARE VARIABLE FOR ALL PROGRAM
 */

/**
 * LoRA
 */
#include <M5LoRa.h>

// Initialise le module LoRa
void initLoraModule(){
    Serial.println("LoRa Duplex Reinitialization");

    // override the default CS, reset, and IRQ pins (optional).  覆盖默认的 CS、复位和 IRQ 引脚（可选）
    LoRa.setPins();// set CS, reset, IRQ pin.  设置 CS、复位、IRQ 引脚

    if (!LoRa.begin(868E6)) {             // initialize ratio at 868 MHz.  868 MHz 时的初始化比率
        Serial.println("LoRa init failed. Check your connections.");
        while (true);                       // if failed, do nothing.  如果失败，什么都不做
    }

    Serial.println("LoRa init succeeded.");
}

// Envoie un message en LoRa
byte LORA_msgCount = 0; // Pour ID unique
byte LORA_localAddress = 0xFF; // ⚠️ ADRESSE DU MICRO-CONTROLLEUR
byte LORA_destination = 0xAF;  // ⚠️ ADRESSE DE DESTINATION
void sendMessage(String payload) {
    LoRa.beginPacket();                   // start packet.
    LoRa.write(LORA_destination);              // add LORA_destination address.
    LoRa.write(LORA_localAddress);             // add sender address.
    LoRa.write(LORA_msgCount);                 // add message ID.
    LoRa.write(payload.length());         // add payload length.
    LoRa.print(payload);                  // add payload.
    LoRa.endPacket();                     // finish packet and send it.
    LORA_msgCount++;                           // increment message ID.
}

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
        if(WebServer_SERVER.hasArg("lora_address") && WebServer_SERVER.hasArg("name") && WebServer_SERVER.hasArg("lora_gateway")) {
            String name = WebServer_SERVER.arg("name");
            String lora_address = WebServer_SERVER.arg("lora_address");
            String lora_gateway = WebServer_SERVER.arg("lora_gateway");
            Serial.println("name :"); Serial.print(name);
            Serial.println("lora_address"); Serial.print(lora_address);
            Serial.println("lora_gateway :"); Serial.print(lora_gateway);
            Serial.println(WebServer_SERVER.hasArg("lora_gateway"));

            Serial.println("strilin lora gateway");
            const char *lora_gateaway_char = lora_gateway.c_str();
            int len = strlen(lora_gateaway_char);
            Serial.println(len);
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
    String ptr = "<!DOCTYPE html><html><head><title>IOT GROUP 19</title><style>*{font-family: Arial,sans-serif;}</style></head><body><h1>IOT WEB SERVER</h1><fieldset><legend>Configuration MODE : <strong id='confMode'></strong></legend><fieldset>If you check \"gateway mode\", you will have to fill in the sensor community project information. <br>Otherwise, you will have to fill in the sensor information and the gateway address.</fieldset><form action='/updateDatas' method='get'><p><label>Name :</label><input name='name' type='text' placeholder='Lora Gateway / Sensor xxx'></p><p><label for='inputIsGateway'>Gateway :</label><input name='inputIsGateway' id='inputIsGateway' type='checkbox' onclick=\"isGateway()\" checked></p><p><label>LoRa Address :</label><input name='lora_address' type='text' placeholder='0xAA'></p><p id='p_lora_gateway' style='display: none'><label>LoRa Gateway :</label><input name='lora_gateway' type='text' placeholder='0xAB'></p><p><input type='submit'></p></form></fieldset><script>isGateway();function isGateway() {if (document.getElementById('inputIsGateway').checked) {document.getElementById('confMode').innerHTML = 'Gateway';document.getElementById('p_lora_gateway').style.display = 'none';}if (!document.getElementById('inputIsGateway').checked) {document.getElementById('confMode').innerHTML = 'Sensor';document.getElementById('p_lora_gateway').style.display = 'block';}}</script></body></html>";
    return ptr;
}

void setup() {
    Serial.begin(115200);

    /**
     * Connection au WIFI CLIENT.
     * Si au bout de 20 secondes la connexion au wifi échoue, on créer l'access point
     */
    tryConnectWifiClient();

    //initLoraModule();
}

void loop(){
    if(WIFI_isStarted && !WebServer_isStarted) {
        initWebServer();
    }

    if(WebServer_isStarted) {
        WebServer_SERVER.handleClient();
    }

}

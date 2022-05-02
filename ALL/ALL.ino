/*#include <M5Stack.h>*/
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h> // https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer
#include <string.h>
#include <HTTPClient.h>

// [GATEWAY & SENSOR] PERSITENT DATAS

// #include "persistentDatas.h"
#include <Preferences.h>
Preferences preferences;
// Récupère les informations stocké en flash
String _NAME = "";
String _LORA_ADDRESS = "";
String _LORA_GATEWAY = "";
String _SSID = "";
String _PASSWORD = "";

void getPersistentDatas() {
    Serial.println("----------- INIT PERSISTENT DATAS -----------");
    preferences.begin("quentin", false);
    _NAME = preferences.getString("name", "");
    _LORA_ADDRESS = preferences.getString("lora_address", "");
    _LORA_GATEWAY = preferences.getString("lora_gateway", "");
    _SSID = preferences.getString("ssid", "");
    _PASSWORD = preferences.getString("password", "");

    Serial.print("SENSOR NAME => "); Serial.println(_NAME);
    Serial.print("_LORA_ADDRESS => "); Serial.println(_LORA_ADDRESS);
    Serial.print("_LORA_GATEWAY => "); Serial.println(_LORA_GATEWAY);
    Serial.print("_SSID => "); Serial.println(_SSID);
    Serial.print("_PASSWORD => "); Serial.println(_PASSWORD);
}

// Récupère le CHIP ID de l'ESP32, requis pour envoyé les données au sensor community project !
uint32_t _CHIP_ID = 0;
void getESPChipID() {
    Serial.println("----------- GET ESP CHIP INFO -----------");
    for(int i=0; i<17; i=i+8) {
        _CHIP_ID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }

    Serial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
    Serial.printf("This chip has %d cores\n", ESP.getChipCores());
    Serial.printf("Chip ID: "); Serial.println(_CHIP_ID);
}

// [GATEWAY & SENSOR] LoRA
#include <M5LoRa.h>
bool LORA_isStarted = false;

// Initialise le module LoRa
void initLoraModule(){
    Serial.println("----------- INIT LORA MODULE -----------");

    // override the default CS, reset, and IRQ pins (optional).  覆盖默认的 CS、复位和 IRQ 引脚（可选）
    LoRa.setPins();// set CS, reset, IRQ pin.  设置 CS、复位、IRQ 引脚

    if (!LoRa.begin(868E6)) {             // initialize ratio at 868 MHz.  868 MHz 时的初始化比率
        Serial.println("LoRa init failed. Check your connections.");
    }

    Serial.println("LoRa init succeeded.");
    LORA_isStarted = true;
}

// Envoie un message en LoRa
byte LORA_msgCount = 0; // Pour ID unique

void sendMessage(String payload) {
    if(LORA_isStarted == false) {
        Serial.println("LORA SEND MESSAGE ERROR : LORA IS NOT INIT");
        return;
    }

    Serial.println("------------- LORA SEND MESSAGE --------------");
    LoRa.beginPacket();                   // start packet.

    // On convertie les adresses en string

    int loraGateway = convertLoraAddress(_LORA_GATEWAY);
    int loraAddress = convertLoraAddress(_LORA_ADDRESS);

    Serial.print("loraGateway => "); Serial.println(loraGateway);
    Serial.print("loraAddress => "); Serial.println(loraAddress);


    LoRa.write(loraGateway);              // add LORA_destination address
    LoRa.write(loraAddress);              // add LORA_ADDRESS address
    LoRa.write(LORA_msgCount);                 // add message ID.
    LoRa.write(payload.length());              // add payload length.
    LoRa.print(payload);                       // add payload.
    LoRa.endPacket();                          // finish packet and send it.
    LORA_msgCount++;                           // increment message ID.
    Serial.println("Fin de l'envoie en LoRa");
}

// Fonction appelé lorsqu'un message LoRa est reçu
void onReceive(int packetSize) {
    Serial.println("------------- LORA RECEIVE MESSAGE --------------");
    if (packetSize == 0 || LORA_isStarted == false) { // Si il n'y a pas de packet ou que le module n'est pas init on s'arrete la
        Serial.println("return");
        return;
    }

    // read packet header bytes:
    int recipient = LoRa.read();          // recipient address.
    byte sender = LoRa.read();            // sender address.
    byte incomingMsgId = LoRa.read();     // incoming msg ID.
    byte incomingLength = LoRa.read();    // incoming msg length.

    String incoming = "";

    while (LoRa.available()) {
        incoming += (char)LoRa.read();
    }

    if (incomingLength != incoming.length()) {   // check length for error.
        Serial.println("error: message length does not match length");
        return;                             // skip rest of function.
    }

    // Vérifie que le message est pour moi ou qu'il est sur l'adresse de broadcast (0xFF)

    // On convertie l'adresse STRING en Byte.
    Serial.print("Recipient =>"); Serial.println(recipient);

    int loraAddress = convertLoraAddress(_LORA_ADDRESS);
    Serial.print("loraAddress =>"); Serial.println(loraAddress);

    if (recipient != loraAddress && recipient != 0xFF) {
        Serial.println("This message is not for me.");
        return;
    }

    // if message is for this device, or broadcast, print details:
    Serial.println("Received from: 0x" + String(sender, HEX));
    Serial.println("Sent to: 0x" + String(recipient, HEX));
    Serial.println("Message ID: " + String(incomingMsgId));
    Serial.println("Message length: " + String(incomingLength));
    Serial.println("Message: " + incoming);
    Serial.println("RSSI: " + String(LoRa.packetRssi()));
    Serial.println("Snr: " + String(LoRa.packetSnr()));
    Serial.println();
}

// Convertie les adresses LoRa stocké dans la flash en Byte.
byte convertLoraAddress(String address) {
    // Serial.println("-------- convertLoraAddress --------");
    //test format : XX char ou 0xXX
    int offset;
    byte convAddr = 0;

    const char* addressChar = address.c_str();
    
    if (strlen(addressChar) == 4 && addressChar[0] == '0' && addressChar[1] == 'x'){
        offset = 2;
    } else {
        if (strlen(addressChar) == 2) {
            offset = 0;
        } else {
            //BAD FORMAT
            return -1;
        }
    }

    // Tu converti le caractère de poids fort en byte
    convAddr += asciiHexToByte(addressChar[offset]);
    convAddr = convAddr << 4;
    convAddr += asciiHexToByte(addressChar[offset+1]);
    // Serial.print("convAddr => "); Serial.println(convAddr);
    return convAddr;
}

byte asciiHexToByte(char c)
{
    Serial.print("char => "); Serial.println(c);
    if(c >= '0' && c <= '9') {
        return c - '0';
    }
    if(c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    if(c>='a' && c <= 'f') {
        return c - 'a' + 10;
    }

    return 0; // ici pour l'exemple genre adresse 0 ou -1 (255 si tu es en non signé) n'est pas une adresse valide et te sert de code d'erreur.
}

// [GATEWAY & SENSOR] WIFI CLIENT & ACCESS POINT
bool WIFI_isStarted = false;

// WIFI CLIENT
//const char* ssid = "Mi 11 Lite";
//const char* password = "quentin1";

// Si au bout de 20 secondes la connexion au wifi échoue, on créer l'access point
void tryConnectWifiClient() {
    if(_SSID == "") {
        startWifiAccessPoint();
    } else {
        WiFi.begin(_SSID.c_str(), _PASSWORD.c_str());
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
}

// WIFI ACCESS POINT [DONNEE EN DURE]
const char* AP_ssid     = "I love my job";
const char* AP_password = "quentin1";
IPAddress AP_local_ip(192,168,3,1);
IPAddress AP_gateway(192,168,3,1);
IPAddress AP_subnet(255,255,255,0);

void startWifiAccessPoint() {
    Serial.println("----------- START WIFI ACCESS POINT -----------");

    Serial.println("\n[*] Creating AP ...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_ssid, AP_password);
    WiFi.softAPConfig(AP_local_ip, AP_gateway, AP_subnet); // Information about access point AP_subnet
    Serial.print("[+] AP Created with IP AP_gateway ");
    Serial.println(WiFi.softAPIP());
    delay(100);
    WIFI_isStarted = true;
}

// [GATEWAY & SENSOR] WEB SERVER
// https://lastminuteengineers.com/creating-esp32-web-server-arduino-ide/

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
            String ssid = WebServer_SERVER.arg("ssid");
            String password = WebServer_SERVER.arg("password");

            // DEBUG
            Serial.println("name :"); Serial.print(name);
            Serial.println("lora_address"); Serial.print(lora_address);
            Serial.println("lora_gateway :"); Serial.print(lora_gateway);
            Serial.println("ssid :"); Serial.print(ssid);
            Serial.println("password :"); Serial.print(password);

            Serial.println(WebServer_SERVER.hasArg("lora_gateway"));

            Serial.println("strlen lora gateway");
            const char *lora_gateaway_char = lora_gateway.c_str();
            int LEN_lora_gateway = strlen(lora_gateaway_char);
            Serial.println(LEN_lora_gateway);


            // Enregistrement des valeurs dans la flash du micro-controller
            Serial.println("Mise en flash du name ...");
            preferences.putString("name", name);

            // FIXME: Ici on pourrais changer et dire que c'est pas un string mais un byte, ça serais surement plsu suimple pour la suite ?!
            preferences.putString("lora_address", lora_address);

            if(LEN_lora_gateway == 0) {
                Serial.println("C'est une gateway");
                // ICI IL FAUT RENTRER LES INFOS DU SENSOR COMMUNITY PROJECT !!!

                // Il faut récupère les informations SSID et PASSWORD
                preferences.putString("ssid", ssid);
                preferences.putString("password", password);
            } else {
                preferences.putString("lora_gateway", lora_gateway);
                Serial.println("C'est le capteur");
            }
        }
    }

    WebServer_SERVER.send(200, "text/html", SendHTML());
    ESP.restart(); // On restart l'ESP si l'utilisateur à remplie de bonnes informations !
}

void WebServer_404(){
    WebServer_SERVER.send(404, "text/plain", "Not found");
}

String SendHTML(){
    String ptr = "<!DOCTYPE html> <html> <head><title>IOT GROUP 19</title>     <style>* {         font-family: Arial, sans-serif;     }</style> </head> <body><h1>IOT WEB SERVER</h1> <fieldset>     <legend>Configuration MODE : <strong id='confMode'></strong></legend>     <fieldset>If you check <em>gateway mode</em>, you will have to fill in the sensor community project information. <br>Otherwise,         you will have to fill in the sensor information and the gateway address.     </fieldset>     <form action='/updateDatas' method='get'><p><label>Name :</label><input name='name' type='text'                                                                             placeholder='Lora Gateway / Sensor xxx'></p>         <p>             <label for='inputIsGateway'>Gateway :</label>             <input name='inputIsGateway' id='inputIsGateway' type='checkbox' onclick='isGateway()' checked>         </p>          <p>             <label>LoRa Address :</label>             <input name='lora_address' type='text' placeholder='0xAA'>         </p>          <div id='sensorFields' style='display: none'>             <p>                 <label>LoRa Gateway :</label>                 <input name='lora_gateway' type='text' placeholder='0xAB'>             </p>         </div>          <div id='gatewayFields' style='display: none'>             <p>                 <label>SSID :</label>                 <input name='ssid' type='text' placeholder='ssid'>             </p>              <p>                 <label>Password :</label>                 <input name='password' type='text' placeholder='password'>             </p>         </div>         <p><input type='submit'></p></form> </fieldset> <script>isGateway();  function isGateway() {     if (document.getElementById('inputIsGateway').checked) {         document.getElementById('confMode').innerHTML = 'Gateway';         document.getElementById('sensorFields').style.display = 'none';         document.getElementById('gatewayFields').style.display = 'block';     }     if (!document.getElementById('inputIsGateway').checked) {         document.getElementById('confMode').innerHTML = 'Sensor';         document.getElementById('sensorFields').style.display = 'block';         document.getElementById('gatewayFields').style.display = 'none';     } }</script> </body> </html> ";
    return ptr;
}


// [GATEWAY] HTTP REQUEST
const char* serverName_DHT22 = "https://api.sensor.community/v1/push-sensor-data/";
const char* endpoints_localApi = "http://192.168.1.74:4443/test";
void sendHTTPRequest_DHT22(float value1, float temperatureFloat, float humidityFloat) {
    // ON ENVOIE LES DATAS AU SERVEUR
    HTTPClient http;

    char* pm1 = "19";
    char* temperature = "22";
    char* humidity = "60";

    // ALL
    char datas[200];
    sprintf(datas, "{\"software_version\":\"%s\",\"sensordatavalues\":[{\"value_type\":\"P1\",\"value\":\"%s\"},{\"value_type\":\"temperature\",\"value\":\"%s\"},{\"value_type\":\"humidity\",\"value\":\"%s\"}]}", "quentin_dev", pm1, temperature, humidity);
    //sprintf(datas, "{\"humidity\":\"%f\",\"temperature\":\"%f\",\"realFeel\":\"%f\",\"value\":\"%s\"}", humidityFloat, temperatureFloat, value1, SENSOR_NAME_DHT22);

    Serial.println(datas);
    http.begin(endpoints_localApi);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Sensor", "esp32-4478969"); // esp32-$_CHIP-ID
    http.addHeader("X-PIN", "1");
    int datasStatus = http.POST(datas);
    Serial.print("HTTP Response code DHT22: ");
    Serial.println(datasStatus);
    http.end();
}


// SETUP & LOOP => PROGRAMM

void setup() {
    Serial.begin(9600);
    getESPChipID();
    getPersistentDatas();
    initLoraModule();

    // Connection au WIFI CLIENT.
    // Si au bout de 20 secondes la connexion au wifi échoue, on créer l'access point
    tryConnectWifiClient();
}

void loop(){
    if(WIFI_isStarted && !WebServer_isStarted) {
        initWebServer();
    }

    if(WebServer_isStarted) {
        WebServer_SERVER.handleClient();
    }

    // Lora RECEIVE
    if(LORA_isStarted) {
        onReceive(LoRa.parsePacket());
    }
    
    Serial.println("Hop, loop finis, on recommence");
    delay(1000);
    //sendHTTPRequest_DHT22(1.1, 2.2, 3.3);
    sendMessage("salut salut");
}

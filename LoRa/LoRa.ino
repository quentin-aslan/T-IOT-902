#include <M5Stack.h>
#include <M5LoRa.h>

String outgoing;

byte msgCount = 0;
byte localAddress = 0xFF; // ⚠️ ADRESSE DU MICRO-CONTROLLEUR
byte destination = 0xFF;  // ⚠️ ADRESSE DE DESTINATION

void setup() {
    M5.begin();
    M5.Power.begin();
    M5.Lcd.println("I love my job");

    initLoraModule();
}

void loop() {

    if(M5.BtnA.wasPressed()){
        pressBtnA();
    }

    if(M5.BtnB.wasPressed()){
        pressBtnB();
    }

    if(M5.BtnC.wasPressed()){
        pressBtnC();
    }

    // parse for a packet, and call onReceive with the result:.  解析数据包，并使用结果调用 onReceive：
    onReceive(LoRa.parsePacket());

    M5.update();
}

// FONCTIONS APPELER LORSQUE L'UTILISATEUR UTILISE LES BOUTONS DU M5

// Bouton à gauche
void pressBtnA() {
    String message = "HeLoRa World!";   // send a message.  发送消息
    sendMessage(message);
    Serial.println("Sending " + message);
    M5.Lcd.setTextColor(BLUE);
    M5.Lcd.println("Sending " + message);
}

// Bouton au milieu
void pressBtnB() {
    initLoraModule();
}

// Bouton à droite
void pressBtnC() {
    clearLCD();
}

// Reinitialise l'écran du M5 (Affiche un écran noir et replace le pointeur au début)
void clearLCD() {
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.clear(BLACK);
}

// Initialise le module LoRa
void initLoraModule(){
    Serial.println("LoRa Duplex Reinitialization");

    // override the default CS, reset, and IRQ pins (optional).  覆盖默认的 CS、复位和 IRQ 引脚（可选）
    LoRa.setPins();// set CS, reset, IRQ pin.  设置 CS、复位、IRQ 引脚

    if (!LoRa.begin(868E6)) {             // initialize ratio at 868 MHz.  868 MHz 时的初始化比率
        Serial.println("LoRa init failed. Check your connections.");
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextColor(RED);
        M5.Lcd.println("Init failed!!!");
        while (true);                       // if failed, do nothing.  如果失败，什么都不做
    }

    M5.Lcd.println("LoRa init succeeded.");
    Serial.println("LoRa init succeeded.");
}

// Envoie un message en LoRa
void sendMessage(String outgoing) {
    LoRa.beginPacket();                   // start packet.
    LoRa.write(destination);              // add destination address.
    LoRa.write(localAddress);             // add sender address.
    LoRa.write(msgCount);                 // add message ID.
    LoRa.write(outgoing.length());        // add payload length.
    LoRa.print(outgoing);                 // add payload.
    LoRa.endPacket();                     // finish packet and send it.
    msgCount++;                           // increment message ID.
}

// Fonction appelé lorsqu'un message LoRa est reçu
void onReceive(int packetSize) {
    if (packetSize == 0) { // Si il n'y a pas de packet on s'arrete la.
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
    if (recipient != localAddress && recipient != 0xFF) {
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

    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.println("Message: " + incoming);
}

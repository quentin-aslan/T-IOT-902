#include <M5Stack.h>
#include <Wire.h>

// AIR QUALITY
#include "Free_Fonts.h"
#include "DFRobot_SHT20.h"

// LORA
#include <M5LoRa.h>

DFRobot_SHT20 sht20;

// Affichage LCD (PM2)
#define X_LOCAL 40
#define Y_LOCAL 30

#define X_OFFSET 160
#define Y_OFFSET 23

// Print the header for a display screen
void header(const char *string, uint16_t color) {
    M5.Lcd.fillScreen(color);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_MAGENTA, TFT_BLUE);
    M5.Lcd.fillRect(0, 0, 320, 30, TFT_BLUE);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.drawString(string, 160, 3, 4);
}

void setup() {
    M5.begin();
    Serial.begin(9600);

    // INIT LCD SCREEN
    M5.Lcd.fillScreen(TFT_BLACK);
    header("P M 2.5", TFT_BLACK);

    initPM2();

    initSHT20();

    initLoraModule();
}

// INIT PM2 QUALITY AIR SENSOR
void initPM2() {
    Serial2.begin(9600, SERIAL_8N1, 16, 17);
    pinMode(13, OUTPUT);
    digitalWrite(13, 1);
}

// INIT FUNCTION

void initSHT20() { // TEMPERATURE AND HUMIDITY
    // INIT SHT20 TEMP & HUMIDITY SENSOR
    sht20.initSHT20();                                  // Init SHT20 Sensor
    delay(100);
    sht20.checkSHT20();
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

#define FRONT 2

// Get values of PM2.5 Air quality
#define PM2_DATAS_LEN 32
uint8_t Air_val[32] = {0};
int16_t p_val[16] = {0};
uint8_t i = 0;

// Temperature, Humidity
float humidity =                0;
float temperature =             0;

// S P M
int16_t SPM_PM1 =               0; // p_val[2]
int16_t SPM_PM25 =              0; // p_val[3]
int16_t SPM_PM10 =              0; // p_val[4]

// A T M E
int16_t ATME_PM1 =              0; // p_val[5]
int16_t ATME_PM25 =             0; // p_val[6]
int16_t ATME_PM10 =             0; // p_val[7]

// number of particules
int16_t _03um =                 0; // p_val[8];
int16_t _05um =                 0; // p_val[9];
int16_t _1um =                  0; // p_val[10];
int16_t _2_5um =                0; // p_val[11];
int16_t _5um =                  0; // p_val[12];
int16_t _10um =                 0; // p_val[13];

void getTempHum() {
    humidity = sht20.readHumidity();                  // Read Humidity
    temperature = sht20.readTemperature();            // Read Temperature
}

void getPM2Values() {
    if (Serial2.available()) {
        Air_val[i] = Serial2.read(); // Lecture des valeurs du PM2.5 (Air Quality)
        Serial.write(Air_val[i]);
        i++;
    } else {
        i = 0;
    }
}

// Une fois que toutes les valeurs ont été récupérer dans la serial, il faut trier les valeurs.
void sortPM2Values() {
    // Pas tout compris a cette algo trouvé sur github sur la doc officiel de M5.
    for (int i = 0, j = 0; i < PM2_DATAS_LEN; i++) {
        if (i % 2 == 0) {
            p_val[j] = Air_val[i];
            p_val[j] = p_val[j] << 8; // On décalle le byte de 8?
        } else {
            p_val[j] |= Air_val[i];
            j++;
        }
    }

    // S P M
    SPM_PM1 =   p_val[2];
    SPM_PM25 =  p_val[3];
    SPM_PM10 =  p_val[4];

    // A T M E
    ATME_PM1 =  p_val[5];
    ATME_PM25 = p_val[6];
    ATME_PM10 = p_val[7];

    // Number of particules
    _03um =     p_val[8];
    _05um =     p_val[9];
    _1um =      p_val[10];
    _2_5um =    p_val[11];
    _5um =      p_val[12];
    _10um =     p_val[13];

}

void LCD_Display_Val(void) {
    M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    M5.Lcd.setCursor(X_LOCAL, Y_LOCAL, FRONT);
    M5.Lcd.print("S P M");

    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

    M5.Lcd.setCursor(X_LOCAL, Y_LOCAL + Y_OFFSET, FRONT);
    M5.Lcd.print("                     ");
    M5.Lcd.setCursor(X_LOCAL, Y_LOCAL + Y_OFFSET, FRONT);
    M5.Lcd.print("PM1.0 : ");
    M5.Lcd.print(SPM_PM1);

    M5.Lcd.setCursor(X_LOCAL, Y_LOCAL + Y_OFFSET * 2, FRONT);
    M5.Lcd.print("                     ");
    M5.Lcd.setCursor(X_LOCAL, Y_LOCAL + Y_OFFSET * 2, FRONT);
    M5.Lcd.print("PM2.5 : ");
    M5.Lcd.print(SPM_PM25);

    M5.Lcd.setCursor(X_LOCAL, Y_LOCAL + Y_OFFSET * 3, FRONT);
    M5.Lcd.print("                     ");
    M5.Lcd.setCursor(X_LOCAL, Y_LOCAL + Y_OFFSET * 3, FRONT);
    M5.Lcd.print("PM10  : ");
    M5.Lcd.print(SPM_PM10);

    M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    M5.Lcd.setCursor(X_LOCAL + X_OFFSET, Y_LOCAL, FRONT);
    M5.Lcd.print("A T M E");

    M5.Lcd.setCursor(X_LOCAL + X_OFFSET, Y_LOCAL + Y_OFFSET, FRONT);
    M5.Lcd.print("                     ");
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setCursor(X_LOCAL + X_OFFSET, Y_LOCAL + Y_OFFSET, FRONT);
    M5.Lcd.print("PM1.0 : ");
    M5.Lcd.print(ATME_PM1);

    M5.Lcd.setCursor(X_LOCAL + X_OFFSET, Y_LOCAL + Y_OFFSET * 2, FRONT);
    M5.Lcd.print("                     ");
    M5.Lcd.setCursor(X_LOCAL + X_OFFSET, Y_LOCAL + Y_OFFSET * 2, FRONT);
    M5.Lcd.print("PM2.5 : ");
    M5.Lcd.print(ATME_PM25);

    M5.Lcd.setCursor(X_LOCAL + X_OFFSET, Y_LOCAL + Y_OFFSET * 3, FRONT);
    M5.Lcd.print("                     ");
    M5.Lcd.setCursor(X_LOCAL + X_OFFSET, Y_LOCAL + Y_OFFSET * 3, FRONT);
    M5.Lcd.print("PM10  : ");
    M5.Lcd.print(ATME_PM10);


    M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    M5.Lcd.setCursor(X_LOCAL + X_OFFSET / 4, Y_LOCAL + Y_OFFSET * 4, FRONT);
    M5.Lcd.print("Number of particles");

    M5.Lcd.setCursor(X_LOCAL, Y_LOCAL + Y_OFFSET * 5, FRONT);
    M5.Lcd.print("                     ");
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setCursor(X_LOCAL, Y_LOCAL + Y_OFFSET * 5, FRONT);
    M5.Lcd.print("0.3um : ");
    M5.Lcd.print(_03um);

    M5.Lcd.setCursor(X_LOCAL, Y_LOCAL + Y_OFFSET * 6, FRONT);
    M5.Lcd.print("                     ");
    M5.Lcd.setCursor(X_LOCAL, Y_LOCAL + Y_OFFSET * 6, FRONT);
    M5.Lcd.print("0.5um : ");
    M5.Lcd.print(_05um);

    M5.Lcd.setCursor(X_LOCAL, Y_LOCAL + Y_OFFSET * 7, FRONT);
    M5.Lcd.print("1.0um : ");
    M5.Lcd.print(_1um);

    M5.Lcd.setCursor(X_LOCAL + X_OFFSET, Y_LOCAL + Y_OFFSET * 5, FRONT);
    M5.Lcd.print("                     ");
    M5.Lcd.setCursor(X_LOCAL + X_OFFSET, Y_LOCAL + Y_OFFSET * 5, FRONT);
    M5.Lcd.print("2.5um : ");
    M5.Lcd.print(_2_5um);

    M5.Lcd.setCursor(X_LOCAL + X_OFFSET, Y_LOCAL + Y_OFFSET * 6, FRONT);
    M5.Lcd.print("                     ");
    M5.Lcd.setCursor(X_LOCAL + X_OFFSET, Y_LOCAL + Y_OFFSET * 6, FRONT);
    M5.Lcd.print("5.0um : ");
    M5.Lcd.print(_5um);

    M5.Lcd.setCursor(X_LOCAL + X_OFFSET, Y_LOCAL + Y_OFFSET * 7, FRONT);
    M5.Lcd.print("                     ");
    M5.Lcd.setCursor(X_LOCAL + X_OFFSET, Y_LOCAL + Y_OFFSET * 7, FRONT);
    M5.Lcd.print("10um  : ");
    M5.Lcd.print(_10um);

    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Lcd.setCursor(X_LOCAL, Y_LOCAL + Y_OFFSET * 8, FRONT);
    M5.Lcd.print("                     ");
    M5.Lcd.setCursor(X_LOCAL, Y_LOCAL + Y_OFFSET * 8, FRONT);
    M5.Lcd.print("T M P : ");
    M5.Lcd.print(temperature);

    M5.Lcd.setCursor(X_LOCAL + X_OFFSET, Y_LOCAL + Y_OFFSET * 8, FRONT);
    M5.Lcd.print("                     ");
    M5.Lcd.setCursor(X_LOCAL + X_OFFSET, Y_LOCAL + Y_OFFSET * 8, FRONT);
    M5.Lcd.print("HUM : ");
    M5.Lcd.print(humidity);

    Serial.println("GetTempHum : ");
    Serial.print(temperature);
    Serial.print(humidity);
}

void loop() {
    if(M5.BtnA.wasPressed()){
        pressBtnA();
    }

    getPM2Values();

    if (i == PM2_DATAS_LEN) {
        getTempHum();
        sortPM2Values();
        LCD_Display_Val();
        String message = "HeLoRa World!";   // send a message.  发送消息
        sendMessage(message);
    }

    if(M5.BtnB.wasPressed()){
        pressBtnB();
    }

    if(M5.BtnC.wasPressed()){
        pressBtnC();
    }
}

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

}

// Bouton à droite
void pressBtnC() {
    
}

// Envoie un message en LoRa
byte msgCount = 0; // Pour ID unique
byte localAddress = 0xFF; // ⚠️ ADRESSE DU MICRO-CONTROLLEUR
byte destination = 0xAF;  // ⚠️ ADRESSE DE DESTINATION
void sendMessage(String payload) {
    LoRa.beginPacket();                   // start packet.
    LoRa.write(destination);              // add destination address.
    LoRa.write(localAddress);             // add sender address.
    LoRa.write(msgCount);                 // add message ID.
    LoRa.write(payload.length());         // add payload length.
    LoRa.print(payload);                  // add payload.
    LoRa.endPacket();                     // finish packet and send it.
    msgCount++;                           // increment message ID.
}

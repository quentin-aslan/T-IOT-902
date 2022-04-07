#include <M5Stack.h>
#include <TinyGPS++.h>


/**
 * INIT GPS
 */
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps; // Librairie GPS.
HardwareSerial ss(2); // Connexion série au GPS.

/**
 * Récupère les valeurs du module GPS et les affiches dans la Serial.
 */
void getGPSDatas() {
    Serial.println("--------------------------------");

    Serial.println("nombre de satellites contacté : ");
    Serial.println(gps.satellites.value());

    Serial.println("Longitude (lng) : ");
    Serial.println(gps.location.lng());

    Serial.println("Latitude (lat) : ");
    Serial.println(gps.location.lat());

    Serial.println("Dernière trames reçu des GPS (ms) : ");
    Serial.println(gps.location.age());

    Serial.println("gps.altitude.isValid() : ");
    Serial.println(gps.altitude.isValid());

    Serial.println("gps.altitude.meters() : ");
    Serial.println(gps.altitude.meters());

    Serial.println("--------------------------------");

    if (millis() > 5000 && gps.charsProcessed() < 10) {
        // LE GPS EST HORS SERVICES !!!
        Serial.println("Aucune datas GPS reçu !");
    }
}

/**
 * Ajoute un delay et s'assure que le module GPS est bien présent.
 * @param ms
 */
static void GPSsmartDelay(unsigned long ms) {
    unsigned long start = millis();
    do {
        while (ss.available()) {
            gps.encode(ss.read());
        }
    } while (millis() - start < ms);
}

void setup() {
    M5.begin();
    M5.Power.begin();
    ss.begin(GPSBaud);

    Serial.begin(9600);
    Serial.println("SETUP FINISH");
}

void loop() {
    M5.Lcd.setCursor(0, 70);
    M5.Lcd.setTextColor(WHITE, BLACK);


    getGPSDatas();
    GPSsmartDelay(1000);
}

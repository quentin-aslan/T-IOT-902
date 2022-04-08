#include <Preferences.h>

Preferences preferences;

void setup() {
    Serial.begin(115200);
    Serial.println();


    preferences.begin("quentin", false);

    // Dans cette exemple j'utilise getUInt mais il y à d'autre fonctions pour manipuler d'autres type de données.
    // https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/

    unsigned int counter = preferences.getUInt("counter", 0);
    counter++;
    Serial.printf("Current counter value: %u\n", counter);
    preferences.putUInt("counter", counter);
    preferences.end();


    Serial.println("Restarting in 10 seconds...");
    delay(10000);

    // Restart ESP
    ESP.restart();
}

void loop() {

}
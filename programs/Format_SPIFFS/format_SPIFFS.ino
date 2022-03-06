#include "SPIFFS.h"


void setup() {
  // Launch SPIFFS file system | Démarre le système de fichier SPIFFS
  Serial.begin(115200);
  delay(100);
 
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  bool formatted = SPIFFS.format();
  if ( formatted ) {
    Serial.println("SPIFFS formatted successfully");
  } else {
    Serial.println("Error formatting");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}

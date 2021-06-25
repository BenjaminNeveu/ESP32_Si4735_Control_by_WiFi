#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <SI4735.h>

#define ESP32_I2C_SDA    21  // I2C bus pin on ESP32
#define ESP32_I2C_SCL    22  // I2C bus pin on ESP32
#define RESET_PIN        12

#define AM_FUNCTION 1
#define FM_FUNCTION 0

uint16_t currentFrequency;
uint16_t previousFrequency;
uint8_t bandwidthIdx = 0;
const char *bandwitdth[] = {"6", "4", "3", "2", "1", "1.8", "2.5"};

int led = 2;

const char *ssid = "RADIO";
const char *password = NULL;

AsyncWebServer server(80);
SI4735 rx;

void setup() {

  pinMode(led, OUTPUT);
  Serial.begin(115200);
  WiFi.softAP(ssid, password);

  //---------------------SPIFFS------------------------

  Serial.println(F("Inizializing FS..."));
  if (SPIFFS.begin()) {
    Serial.println(F("SPIFFS mounted correctly."));
  } else {
    Serial.println(F("!An error occurred during SPIFFS mounting"));
  }

  unsigned int totalBytes = SPIFFS.totalBytes();
  unsigned int usedBytes = SPIFFS.usedBytes();

  Serial.println();
  Serial.println("===== File system info =====");

  Serial.print("Total space:      ");
  Serial.print(totalBytes);
  Serial.println("byte");

  Serial.print("Total space used: ");
  Serial.print(usedBytes);
  Serial.println("byte");

  Serial.println();
  // Open dir folder
  File dir = SPIFFS.open("/");
  // List file at root
  listFilesInDir(dir, 1);

  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  //-------------------SI4735---------------------------

  Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);
  delay(500);
  // Look for the Si47XX I2C bus address
  int16_t si4735Addr = rx.getDeviceI2CAddress(RESET_PIN);
  if ( si4735Addr == 0 ) {
    Serial.println("Si473X not found!");
    Serial.flush();
    while (1);
  } else {
    Serial.print("The Si473X I2C address is 0x");
    Serial.println(si4735Addr, HEX);
  }

  delay(500);
  rx.setup(RESET_PIN, FM_FUNCTION);
  // rx.setup(RESET_PIN, -1, 1, SI473X_ANALOG_AUDIO);
  // Starts defaul radio function and band (FM; from 84 to 108 MHz; 103.9 MHz; step 100kHz)
  rx.setFM(8400, 10800, 10550, 10);
  delay(500);
  currentFrequency = previousFrequency = rx.getFrequency();
  rx.setVolume(25);

  //-------------------------SERVER-FILE-----------------------

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {

    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/fonction.js", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/fonction.js", "text/javascript");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/jquery.min.js", "text/javascript");
  });

//-------------------------SERVER-REQUEST-----------------------

  server.on("/mode", HTTP_POST, [](AsyncWebServerRequest * request) {
    AsyncWebParameter* p = request->getParam(0);
    if (p->value() == "fm") {
      rx.setFM(8600, 10800, 10550 , 50);
      rx.setSeekAmRssiThreshold(0);
      rx.setSeekAmSrnThreshold(10);
    }
    if (p->value() == "am") {
      rx.setAM(520, 1750, 810, 10);
      rx.setSeekAmLimits(520, 1750);
      rx.setSeekAmSpacing(10);
    }
    request->send(200);
  });

  server.on("/getFrequence", HTTP_GET, [](AsyncWebServerRequest * request){   
    previousFrequency = currentFrequency = rx.getFrequency();
    if (rx.isCurrentTuneFM()) {
      String frequence = (String(currentFrequency / 100.0, 2));
      request->send(200, "text/plain", frequence);
    } else {
      String frequence = (String(currentFrequency));
      request->send(200, "text/plain", frequence);
    }
  });

  server.on("/getSnr", HTTP_GET, [](AsyncWebServerRequest * request){
    rx.getCurrentReceivedSignalQuality();
    String snr = (String(rx.getCurrentSNR()));
    request->send(200, "text/plain", snr);
  });

  server.on("/getRssi", HTTP_GET, [](AsyncWebServerRequest * request){
    rx.getCurrentReceivedSignalQuality();
    String rssi = (String(rx.getCurrentRSSI()));
    request->send(200, "text/plain", rssi);
  });

  server.on("/getPilot", HTTP_GET, [](AsyncWebServerRequest * request){
    rx.getCurrentReceivedSignalQuality();
     if (rx.isCurrentTuneFM()){
    String rssi = ((rx.getCurrentPilot()) ? "STEREO" : "MONO");
    request->send(200, "text/plain", rssi);
  }else{
    String rssi = (String(""));
    request->send(200, "text/plain", rssi);
  }});

  server.on("/volume", HTTP_POST, [](AsyncWebServerRequest * request) {
    AsyncWebParameter* p = request->getParam(0);
    int val = p->value().toInt();
    rx.setVolume(val);
    request->send(200);
  });

  server.on("/frequence", HTTP_POST, [](AsyncWebServerRequest * request) {
    AsyncWebParameter* p = request->getParam(0);
    int val = p->value().toInt();
    rx.setFrequency(val);
    request->send(200);
  });

  server.on("/up", HTTP_POST, [](AsyncWebServerRequest * request) {
    AsyncWebParameter* p = request->getParam(0);
    int step = p->value().toInt();
    rx.setFrequencyStep(step);
    rx.frequencyUp();
    request->send(200);
  });

  server.on("/down", HTTP_POST, [](AsyncWebServerRequest * request) {
    AsyncWebParameter* p = request->getParam(0);
    int step = p->value().toInt();
    rx.setFrequencyStep(step);
    rx.frequencyDown();
    request->send(200);
  });

  server.on("/seekdown", HTTP_GET, [](AsyncWebServerRequest * request) {
    rx.seekStationProgress(showFrequency, 0);
    request->send(200);
  });

  server.on("/seekup", HTTP_GET, [](AsyncWebServerRequest * request) {
    rx.seekStationProgress(showFrequency, 1);
    request->send(200);
  });

  server.on("/bandwidth", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (bandwidthIdx > 6)
      bandwidthIdx = 0;
    rx.setBandwidth(bandwidthIdx, 1);
    bandwidthIdx++;
    request->send(200);
  });

  server.begin();

}

void showFrequency( uint16_t freq ) {

  if (rx.isCurrentTuneFM())
  {
    Serial.print(String(freq / 100.0, 2));
    Serial.println("MHz ");
  }
  else
  {
    Serial.print(freq);
    Serial.println("kHz");
  }

}

void listFilesInDir(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files in the folder
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      listFilesInDir(entry, numTabs + 1);
    } else {
      // display zise for file, nothing for directory
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

void loop() {

}

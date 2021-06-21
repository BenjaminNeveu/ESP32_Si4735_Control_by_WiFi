#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <SI4735.h>
// Test it with patch_init.h or patch_full.h. Do not try load both.
#include "patch_init.h" // SSB patch for whole SSBRX initialization string
// #include "patch_full.h"    // SSB patch for whole SSBRX full download

const uint16_t size_content = sizeof ssb_patch_content; // see ssb_patch_content in patch_full.h or patch_init.h

#define ESP32_I2C_SDA    21  // I2C bus pin on ESP32
#define ESP32_I2C_SCL    22  // I2C bus pin on ESP32
#define RESET_PIN        12

#define AM_FUNCTION 1
#define LSB 1
#define USB 2

bool disableAgc = true;
bool avc_en = true;

int currentBFO = 0;

uint16_t currentFrequency;
uint8_t currentStep = 1;
uint8_t currentBFOStep = 25;

uint8_t bandwidthIdx = 2;
const char *bandwitdth[] = {"1.2", "2.2", "3.0", "4.0", "0.5", "1.0"};

long et1 = 0, et2 = 0;

typedef struct
{
  uint16_t minimumFreq;
  uint16_t maximumFreq;
  uint16_t currentFreq;
  uint16_t currentStep;
  uint8_t currentSSB;
} Band;

Band band[] = {
  {472, 479, 472, 1, LSB},//630
  {1800, 2000, 1810, 1, LSB},//160
  {3500, 4000, 3700, 1, LSB},//80
  {5200, 5405, 5200, 1, LSB},//60
  {7000, 7300, 7100, 1, LSB},//40
  {10100, 10150, 10100, 1, USB},//30
  {14000, 14350, 14070, 1, USB},//20
  {18068, 18168, 18100, 1, USB},//17
  {21000, 21450, 21200, 1, USB},//15
  {24890, 24990, 24940, 1, USB},//12
  {28000, 29700, 28400, 1, USB}//10
};

const int lastBand = (sizeof band / sizeof(Band)) - 1;
// int currentFreqIdx = 9;
int currentFreqIdx = 5;
uint8_t currentAGCAtt = 0;

uint8_t rssi = 0;

const char *ssid = "RADIO";
const char *password = NULL;

AsyncWebServer server(80);

SI4735 si4735;

void setup()
{

  Serial.begin(115200);
  WiFi.softAP(ssid, password);

  //-------------------SI4735---------------------------

  Serial.println("Si4735 Arduino Library");
  Serial.println("SSB TEST");

  // The line below may be necessary to setup I2C pins on ESP32
  Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);
  delay(500);

  // Gets and sets the Si47XX I2C bus address
  int16_t si4735Addr = si4735.getDeviceI2CAddress(RESET_PIN);
  if ( si4735Addr == 0 ) {
    Serial.println("Si473X not found!");
    Serial.flush();
    while (1);
  } else {
    Serial.print("The Si473X I2C address is 0x");
    Serial.println(si4735Addr, HEX);
  }

  si4735.setup(RESET_PIN, AM_FUNCTION);

  // Testing I2C clock speed and SSB behaviour
  // si4735.setI2CLowSpeedMode();     //  10000 (10kHz)
  // si4735.setI2CStandardMode();     // 100000 (100kHz)
  // si4735.setI2CFastMode();         // 400000 (400kHz)
  // si4735.setI2CFastModeCustom(500000); // -> It is not safe and can crash.
  delay(10);
  Serial.println("SSB patch is loading...");
  et1 = millis();
  loadSSB();
  et2 = millis();
  Serial.print("SSB patch was loaded in: ");
  Serial.print( (et2 - et1) );
  Serial.println("ms");
  delay(100);
  si4735.setTuneFrequencyAntennaCapacitor(1); // Set antenna tuning capacitor for SW.
  si4735.setSSB(band[currentFreqIdx].minimumFreq, band[currentFreqIdx].maximumFreq, band[currentFreqIdx].currentFreq, band[currentFreqIdx].currentStep, band[currentFreqIdx].currentSSB);
  delay(100);
  currentFrequency = si4735.getFrequency();
  si4735.setVolume(35);

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

  server.on("/jquery.rotaryswitch.js", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/jquery.rotaryswitch.js", "text/javascript");
  });

  server.on("/jquery.rotaryswitch.css", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/jquery.rotaryswitch.css", "text/css");
  });

  server.on("/darkSmallFront.png", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/darkSmallFront.png", "image/png");
  });

  server.on("/darkSmallBackground.png", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/darkSmallBackground.png", "image/png");
  });

  server.on("/ref.png", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/ref.png", "image/png");
  });

  //-------------------------SERVER-REQUEST-----------------------

  server.on("/mode", HTTP_POST, [](AsyncWebServerRequest * request) {

    AsyncWebParameter* p = request->getParam(0);
    si4735.setTuneFrequencyAntennaCapacitor(1); // Set antenna tuning capacitor for SW.
    si4735.setSSB(band[currentFreqIdx].minimumFreq, band[currentFreqIdx].maximumFreq, band[currentFreqIdx].currentFreq, band[currentFreqIdx].currentStep, band[currentFreqIdx].currentSSB);
    if (p->value() == "usb") {
      band[currentFreqIdx].currentSSB  = 2;
    }
    if (p->value() == "lsb") {
      band[currentFreqIdx].currentSSB = 1;
    }
    request->send(200);

  });

  server.on("/getFrequence", HTTP_GET, [](AsyncWebServerRequest * request) {
    String freqDisplay;
    currentFrequency = si4735.getFrequency();
    freqDisplay = String((float)currentFrequency / 1000, 3);
    request->send(200, "text/plain", freqDisplay);
  });

  server.on("/getSnr", HTTP_GET, [](AsyncWebServerRequest * request) {
    si4735.getCurrentReceivedSignalQuality();
    String snr = (String(si4735.getCurrentSNR()));
    request->send(200, "text/plain", snr);
  });

  server.on("/getRssi", HTTP_GET, [](AsyncWebServerRequest * request) {
    si4735.getCurrentReceivedSignalQuality();
    String rssi = (String(si4735.getCurrentRSSI()));
    request->send(200, "text/plain", rssi);
  });

  server.on("/getAgc", HTTP_GET, [](AsyncWebServerRequest * request) {
    si4735.getAutomaticGainControl();
    String agc = ((si4735.isAgcEnabled()) ? "AGC&nbsp;ON&nbsp;&nbsp;&nbsp;&nbsp;" : "AGC&nbsp;OFF&nbsp;&nbsp;&nbsp;");
    request->send(200, "text/plain", agc);

  });

  server.on("/getBfo", HTTP_GET, [](AsyncWebServerRequest * request) {
    String bfo;
    if (currentBFO > 0) {
      bfo = "+" + String(currentBFO);
    } else {
      bfo = String(currentBFO);
    }
    request->send(200, "text/plain", bfo);
  });

  server.on("/volume", HTTP_POST, [](AsyncWebServerRequest * request) {
    AsyncWebParameter* p = request->getParam(0);
    int val = p->value().toInt();
    si4735.setVolume(val);
    request->send(200);
  });

  server.on("/frequence", HTTP_POST, [](AsyncWebServerRequest * request) {
    AsyncWebParameter* p = request->getParam(0);
    int val = p->value().toInt();
    si4735.setFrequency(val);
    request->send(200);
  });

  server.on("/upFreq", HTTP_POST, [](AsyncWebServerRequest * request) {
    AsyncWebParameter* p = request->getParam(0);
    currentStep = p->value().toInt();
    si4735.setFrequencyStep(currentStep);
    band[currentFreqIdx].currentStep = currentStep;
    si4735.frequencyUp();
    band[currentFreqIdx].currentFreq = currentFrequency = si4735.getCurrentFrequency();
    request->send(200);
  });

  server.on("/downFreq", HTTP_POST, [](AsyncWebServerRequest * request) {
    AsyncWebParameter* p = request->getParam(0);
    currentStep = p->value().toInt();
    si4735.setFrequencyStep(currentStep);
    band[currentFreqIdx].currentStep = currentStep;
    si4735.frequencyDown();
    band[currentFreqIdx].currentFreq = currentFrequency = si4735.getCurrentFrequency();
    request->send(200);
  });
  
  server.on("/upBfo", HTTP_POST, [](AsyncWebServerRequest * request) {
    AsyncWebParameter* p = request->getParam(0);
    int val = p->value().toInt();
    currentBFOStep = val;
    currentBFO += currentBFOStep;
    si4735.setSSBBfo(currentBFO);
    request->send(200);
  });

  server.on("/downBfo", HTTP_POST, [](AsyncWebServerRequest * request) {
    AsyncWebParameter* p = request->getParam(0);
    int val = p->value().toInt();
    currentBFOStep = val;
    currentBFO -= currentBFOStep;
    si4735.setSSBBfo(currentBFO);
    request->send(200);
  });

  server.on("/bandwidth", HTTP_POST, [](AsyncWebServerRequest * request) {
    AsyncWebParameter* p = request->getParam(0);
    int val = p->value().toInt();
    bandwidthIdx = val;
    si4735.setSSBAudioBandwidth(bandwidthIdx);
    if (bandwidthIdx == 0 || bandwidthIdx == 4 || bandwidthIdx == 5) {
      si4735.setSBBSidebandCutoffFilter(0);
    } else {
      si4735.setSBBSidebandCutoffFilter(1);
    }
    request->send(200);
  });

  server.on("/ham", HTTP_POST, [](AsyncWebServerRequest * request) {
    AsyncWebParameter* p = request->getParam(0);
    band[currentFreqIdx].currentFreq = currentFrequency;
    int val = p->value().toInt();
    int lastSSB = band[currentFreqIdx].currentSSB;
    currentFreqIdx = val;
    si4735.setTuneFrequencyAntennaCapacitor(1); // Set antenna tuning capacitor for SW.
    si4735.setSSB(band[currentFreqIdx].minimumFreq, band[currentFreqIdx].maximumFreq, band[currentFreqIdx].currentFreq, band[currentFreqIdx].currentStep, band[currentFreqIdx].currentSSB);
    delay(250);
    band[currentFreqIdx].currentSSB = lastSSB;
    request->send(200);

  });

  server.on("/agc", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("agc");
    disableAgc = !disableAgc;
    si4735.setAutomaticGainControl(disableAgc, currentAGCAtt);
    request->send(200);

  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest * request) {
    currentBFO = 0;
    si4735.setSSBBfo(currentBFO);
    request->send(200);
  });

  server.begin();

}


/*
   This function loads the contents of the ssb_patch_content array into the CI (Si4735) and starts the radio on
   SSB mode.
*/
void loadSSB()
{
  si4735.setI2CFastModeCustom(500000);
  si4735.queryLibraryId(); // Is it really necessary here? I will check it.
  si4735.patchPowerUp();
  delay(50);
  si4735.downloadPatch(ssb_patch_content, size_content);
  // Parameters
  // AUDIOBW - SSB Audio bandwidth; 0 = 1.2kHz (default); 1=2.2kHz; 2=3kHz; 3=4kHz; 4=500Hz; 5=1kHz;
  // SBCUTFLT SSB - side band cutoff filter for band passand low pass filter ( 0 or 1)
  // AVC_DIVIDER  - set 0 for SSB mode; set 3 for SYNC mode.
  // AVCEN - SSB Automatic Volume Control (AVC) enable; 0=disable; 1=enable (default).
  // SMUTESEL - SSB Soft-mute Based on RSSI or SNR (0 or 1).
  // DSP_AFCDIS - DSP AFC Disable or enable; 0=SYNC MODE, AFC enable; 1=SSB MODE, AFC disable.
  si4735.setSSBConfig(bandwidthIdx, 1, 0, 1, 0, 1);
  si4735.setI2CFastModeCustom(100000);
}

/*
   Main
*/
void loop()
{ /*
    // Check if exist some command to execute
    if (Serial.available() > 0)
    {
     char key = Serial.read();

     else if (key == 'G' || key == 'g') // switches on/off the Automatic Gain Control
     {
       disableAgc = !disableAgc;
       // siwtch on/off ACG; AGC Index = 0. It means Minimum attenuation (max gain)
       si4735.setAutomaticGainControl(disableAgc, currentAGCAtt);
       showStatus();
     }
  */

  /*else if (key == 'A' || key == 'a') // Switches the LNA Gain index  attenuation
    {
    if (currentAGCAtt == 0)
      currentAGCAtt = 1;
    else if (currentAGCAtt == 1)
      currentAGCAtt = 5;
    else if (currentAGCAtt == 5)
      currentAGCAtt = 15;
    else if (currentAGCAtt == 15)
      currentAGCAtt = 26;
    else
      currentAGCAtt = 0;
    si4735.setAutomaticGainControl(1 /*disableAgc*//*, currentAGCAtt);
  showStatus();
  }
*/
  /*
    else if (key == 'C' || key == 'c') // switches on/off the Automatic Volume Control
    {
      avc_en = !avc_en;
      si4735.setSSBAutomaticVolumeControl(avc_en);
    }
    delay(200);*/
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

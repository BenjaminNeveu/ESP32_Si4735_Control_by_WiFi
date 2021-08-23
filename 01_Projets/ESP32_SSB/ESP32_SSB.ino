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

const char *ssid = "Récepteur SSB";
const char *password = NULL;

AsyncWebServer server(80);

bool disableAgc = true;
bool avc_en = true;

int currentBFO = 0;

uint16_t currentFrequency;
uint8_t currentMode;
uint8_t currentStep = 1;
uint8_t currentBFOStep = 1;

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
  // freq-min, freq-max, freq, step-freq, modeSSB
  {472, 479, 472, 1, LSB},//630
  {1810, 1850, 1810, 1, LSB},//160
  {3500, 3800, 3700, 1, LSB},//80
  {5350, 5366, 5200, 1, LSB},//60
  {7000, 7200, 7100, 1, LSB},//40
  {10100, 10150, 10100, 1, USB},//30
  {14000, 14350, 14070, 1, USB},//20
  {18068, 18168, 18100, 1, USB},//17
  {21000, 21450, 21200, 1, USB},//15
  {24890, 24990, 24940, 1, USB},//12
  {28000, 29700, 28400, 1, USB}//10
};

const int lastBand = (sizeof band / sizeof(Band)) - 1;
int currentFreqIdx = 5;
uint8_t currentAGCAtt = 0;

uint8_t rssi = 0;



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
    si4735.setTuneFrequencyAntennaCapacitor(1);
    currentFreqIdx = 5;
    si4735.setSSB(band[currentFreqIdx].minimumFreq, band[currentFreqIdx].maximumFreq, band[currentFreqIdx].currentFreq, band[currentFreqIdx].currentStep, band[currentFreqIdx ].currentSSB);
    si4735.setFrequency(10100);
    band[currentFreqIdx].currentFreq = currentFrequency = si4735.getCurrentFrequency();
    si4735.setSSBBfo(0);
    si4735.setVolume(35);
  });

  server.on("/fonction.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/fonction.js", "text/javascript");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/jquery.min.js", "text/javascript");
  });

  server.on("/jquery.rotaryswitch.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/jquery.rotaryswitch.js", "text/javascript");
  });

  server.on("/jquery.rotaryswitch.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/jquery.rotaryswitch.css", "text/css");
  });

  server.on("/darkSmallFront.png", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/darkSmallFront.png", "image/png");
  });

  server.on("/darkSmallBackground.png", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/darkSmallBackground.png", "image/png");
  });

  server.on("/ref.png", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/ref.png", "image/png");
  });

  //-------------------------SERVER-REQUEST-----------------------

  //-------------------Actualisation des données---------------------------

  // recupére la fréquence pour la mettre a jour
  server.on("/getFrequency", HTTP_GET, [](AsyncWebServerRequest * request) {
    // initialisation de la variable
    String freqDisplay;
    // met a jours la fréquence
    currentFrequency = si4735.getFrequency();
    // récupere la valeur de la fréquence et le met en chaine de caractére
    freqDisplay = String((float)currentFrequency / 1000, 3);
    // envoie de la chaine caractere
    request->send(200, "text/plain", freqDisplay);
  });

  // recupére le SNR pour le mettre a jour
  server.on("/getSnr", HTTP_GET, [](AsyncWebServerRequest * request) {
    si4735.getCurrentReceivedSignalQuality();
    // récupere la valeur du SNR et le met en chaine de caractére
    String snr = (String(si4735.getCurrentSNR()));
    // envoie de la chaine caractere
    request->send(200, "text/plain", snr);
  });

  // recupére le RSSI pour le mettre a jour
  server.on("/getRssi", HTTP_GET, [](AsyncWebServerRequest * request) {
    si4735.getCurrentReceivedSignalQuality(); 
    // récupere la valeur du RSSI et le met en chaine de caractére
    String rssi = (String(si4735.getCurrentRSSI()));
    // envoie de la chaine caractere
    request->send(200, "text/plain", rssi);
  });

  // recupére l'etat de l'AGC pour le mettre a jour
  server.on("/getAgc", HTTP_GET, [](AsyncWebServerRequest * request) {
    si4735.getAutomaticGainControl();
    // isAgcEnabled renvoie true ou false 
    // on renvoie donc "AGC ON" ou "AGC OFF" sous la forme d'une chaine de caractére
    String agc = ((si4735.isAgcEnabled()) ? "AGC&nbsp;ON&nbsp;&nbsp;&nbsp;&nbsp;" : "AGC&nbsp;OFF&nbsp;&nbsp;&nbsp;");
    // envoie de la chaine caractere
    request->send(200, "text/plain", agc);
  });

  // recupére le BFO pour le mettre a jour
  server.on("/getBfo", HTTP_GET, [](AsyncWebServerRequest * request) {
    String bfo;
    // le bfo pouvant etre positif ou negatif
    if (currentBFO > 0) {
      // par défaut currnetBFO ne prend pas de '+' quand il est positif mais prend un '-' quand il est négatif
      // ajout d'un + si > 0
      bfo = "+" + String(currentBFO);
    } else {
      bfo = String(currentBFO);
    }
    // envoie de la chaine caractere
    request->send(200, "text/plain", bfo);
  });

  //-------------------Control utilisateur---------------------
    
  server.on("/mode", HTTP_POST, [](AsyncWebServerRequest * request) {
    //je recupére la valeur placer en parametre lors de la requete
    AsyncWebParameter* p = request->getParam(0);
    si4735.setTuneFrequencyAntennaCapacitor(1); // Set antenna tuning capacitor for SW.
    // par default le mode n'est pas selectionable car il est paramètrer via le choix de band
    // currentSSB ne peut prendre que 2 valeurs 1 ou 2 (lsb ou usb)
    if (p->value() == "usb") {
      // dans le tableau band le currentSSB est egale 'usb' pour l'id 5
      si4735.setSSB(band[currentFreqIdx].minimumFreq, band[currentFreqIdx].maximumFreq, band[currentFreqIdx].currentFreq, band[currentFreqIdx].currentStep, band[5].currentSSB);
      currentMode = 5;
    }
    if (p->value() == "lsb") {
      // dans le tableau band le currentSSB est egale 'lsb' pour l'id 1
      si4735.setSSB(band[currentFreqIdx].minimumFreq, band[currentFreqIdx].maximumFreq, band[currentFreqIdx].currentFreq, band[currentFreqIdx].currentStep, band[1].currentSSB);
      currentMode = 1;
    }
    request->send(200);
  });
  
  server.on("/volume", HTTP_POST, [](AsyncWebServerRequest * request) {
    //recupere la valeur placer en parametre dans l'url
    AsyncWebParameter* p = request->getParam(0);
    //inscrit la valeur de la variable 'p' dans la variable val en int
    int val = p->value().toInt();
    // parametre la volume avec la variable val
    // le volume peut être compris entre 0 et 63
    si4735.setVolume(val);
    request->send(200);
  });

  server.on("/frequency", HTTP_POST, [](AsyncWebServerRequest * request) {
    //recupere la valeur placer en parametre dans l'url
    AsyncWebParameter* p = request->getParam(0);
    //inscrit la valeur dans la variable val
    int freq = p->value().toInt();
    // parametre la frequence avec la variable freq
    si4735.setFrequency(freq);
    request->send(200);
  });

  server.on("/upFreq", HTTP_POST, [](AsyncWebServerRequest * request) {
    // recupere la valeur placer en parametre dans l'url qui correspond au select 'step-freq' de la page html
    AsyncWebParameter* p = request->getParam(0);
    // inscrit la valeur dans la variable currentStep
    currentStep = p->value().toInt();
    // met a jour la valeur du step
    si4735.setFrequencyStep(currentStep);
    band[currentFreqIdx].currentStep = currentStep;
    // augmente le frequence
    si4735.frequencyUp();
    band[currentFreqIdx].currentFreq = currentFrequency = si4735.getCurrentFrequency();
    request->send(200);
  });

  server.on("/downFreq", HTTP_POST, [](AsyncWebServerRequest * request) {
    // recupere la valeur placer en parametre dans l'url qui correspond au select 'step-freq' de la page html
    AsyncWebParameter* p = request->getParam(0);
    // convertion de la chaine de caractere 'p' en int
    // inscrit la valeur dans la variable val
    currentStep = p->value().toInt();
    // met a jour la valeur du step
    si4735.setFrequencyStep(currentStep);
    band[currentFreqIdx].currentStep = currentStep;
    // diminue la frequence
    si4735.frequencyDown();
    band[currentFreqIdx].currentFreq = currentFrequency = si4735.getCurrentFrequency();
    request->send(200);
  });

  server.on("/upBfo", HTTP_POST, [](AsyncWebServerRequest * request) {
    // recupere la valeur placer en parametre dans l'url qui correspond au select 'step-bfo' de la page html
    AsyncWebParameter* p = request->getParam(0);
    // convertion de la chaine de caractere 'p' en int
    // inscrit la valeur dans la variable currentBFOStep
    currentBFOStep = p->value().toInt();
    // recupere la valeur de currnetBFO et ajoute currentBFOStep a celle-ci
    currentBFO += currentBFOStep;
    si4735.setSSBBfo(currentBFO);
    request->send(200);
  });

  server.on("/downBfo", HTTP_POST, [](AsyncWebServerRequest * request) {
    // recupere la valeur placer en parametre dans l'url qui correspond au select 'step-bfo' de la page html
    AsyncWebParameter* p = request->getParam(0);
    // convertion de la chaine de caractere 'p' en int
    // inscrit la valeur dans la variable currentBFOStep
    currentBFOStep = p->value().toInt();
    // recupere la valeur de currnetBFO et ajoute currentBFOStep a celle-ci
    currentBFO -= currentBFOStep;
    si4735.setSSBBfo(currentBFO);
    request->send(200);
  });

  server.on("/bandwidth", HTTP_POST, [](AsyncWebServerRequest * request) {
    // recupere la valeur placer en parametre dans l'url qui correspond au select 'BandW' de la page html
    AsyncWebParameter* p = request->getParam(0);
    // convertion de la chaine de caractere 'p' en int
    // inscrit la valeur dans la variable bandwidthIdx
    bandwidthIdx = p->value().toInt();
    si4735.setSSBAudioBandwidth(bandwidthIdx);
    if (bandwidthIdx == 0 || bandwidthIdx == 4 || bandwidthIdx == 5) {
      si4735.setSBBSidebandCutoffFilter(0);
    } else {
      si4735.setSBBSidebandCutoffFilter(1);
    }
    request->send(200);
  });

  server.on("/ham", HTTP_POST, [](AsyncWebServerRequest * request) {
    // recupere la valeur placer en parametre dans l'url qui correspond au select 'ham' de la page html
    AsyncWebParameter* p = request->getParam(0);
    // convertion de la chaine de caractere 'p' en int
    // inscrit la valeur dans la variable currentFreqIdx
    currentFreqIdx = p->value().toInt();
    si4735.setTuneFrequencyAntennaCapacitor(1); // Set antenna tuning capacitor for SW.
    // on applique currentFreqIdx dans la struct 'band' pour mettre à jour la bande de fréquence utilisé
    si4735.setSSB(band[currentFreqIdx].minimumFreq, band[currentFreqIdx].maximumFreq, band[currentFreqIdx].currentFreq, band[currentFreqIdx].currentStep, band[currentMode].currentSSB);
    request->send(200);

  });

  
  server.on("/agc", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("agc");
    // AGC ne pouvant prendre que deux valeurs 'true' ou 'false'
    // on passe donc de 'true' a 'false' et inversement a chaque fois que cette instruction est lancé
    disableAgc = !disableAgc;
    // application du changement
    si4735.setAutomaticGainControl(disableAgc, currentAGCAtt);
    request->send(200);

  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest * request) {
    // je met le le BFO a 0 pour le réinitialiser
    si4735.setSSBBfo(0);
    request->send(200);
  });

  server.begin();

}

/*
   Main
*/
void loop() {
   
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

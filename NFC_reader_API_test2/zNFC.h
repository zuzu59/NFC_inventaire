// zf240609.1757


/*
 * 
 * Pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      ESP32-c3
 *             Reader/PCB   super mini
 * Signal      Pin          Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          GPIO10
 * SPI SS      SDA(SS)      GPIO7
 * SPI MOSI    MOSI         GPIO6
 * SPI MISO    MISO         GPIO5
 * SPI SCK     SCK          GPIO4
 *
 */



// API JSON
#include <ArduinoJson.h>
const char* zToken = apiToken;
long zIndex = 0;

const char* apiGetIndexToto = apiServerName "/api/v2/tables/mccwrj43jwtogvs/records?viewId=vwwm6yz0uhytc9er&fields=Index&sort=-Index&limit=1&shuffle=0&offset=0";
const char* apiPostNewRecordToto = apiServerName "/api/v2/tables/mccwrj43jwtogvs/records";

const char* apiGetIndexTagCmd = apiServerName "/api/v2/tables/mmkk01cafw4ynyp/records?viewId=vw68oujklglmmlp3&fields=Index&sort=-Index&limit=1&shuffle=0&offset=0";
const char* apiPostNewRecordTagCmd = apiServerName "/api/v2/tables/mmkk01cafw4ynyp/records";

const char* apiGetRfidTagCmd = apiServerName "/api/v2/tables/mmkk01cafw4ynyp/records?viewId=vw68oujklglmmlp3&where=%28UID%20RFID%2Ceq%2Cxxx%29&limit=25&shuffle=0&offset=0";

const char* apiGetIndexTagLog = apiServerName "/api/v2/tables/md736jl0ppzh1jj/records?viewId=vwl66xl4gwk919f1&fields=Index&sort=-Index&limit=1&shuffle=0&offset=0";
const char* apiPostNewRecordTagLog = apiServerName "/api/v2/tables/md736jl0ppzh1jj/records";




String getToDB(String zApigetToDB) {
  // Efectue la requête GET pour récupérer l'enregistrement
  http.begin(zApigetToDB);
  http.addHeader("accept", "application/json");
  http.addHeader("xc-token", zToken);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    http.end();
    return(payload);
  }
  http.end();
  String zError = "Error on HTTP request in getToDB: " + String(httpCode);
  USBSerial.println(zError);
  return(zError);
}



void getIndex(String payload){
  // Alloue un objet DynamicJsonDocument pour stocker le JSON
  DynamicJsonDocument doc(1024);
  // Désérialise le JSON
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    USBSerial.print("deserializeJson() failed: ");
    USBSerial.println(error.f_str());
    return;
  }
  // Récupère le champ "Index" et l'incrémente
  zIndex = doc["list"][0]["Index"].as<long>() + 1;
  USBSerial.print("Index incremented: ");
  USBSerial.println(zIndex);
}




static void postToDB(String zApiPostToDb, String jsonReqBody) {
  // Effectuer la requête POST pour créer le nouvel enregistrement
  http.begin(zApiPostToDb);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("xc-token", zToken);
  int httpCode = http.POST(jsonReqBody);
  if (httpCode > 0) {
    USBSerial.printf("POST request response code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      String response = http.getString();
      USBSerial.println("POST request response:");
      USBSerial.println(response);
    }
  } else {
    USBSerial.println("Error on POST request");
  }
  http.end();
}






// RFID MFRC522
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 7
#define RST_PIN 10
 
MFRC522 rfid(SS_PIN, RST_PIN);  // Instance of the class
MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[7];

// start RFID
void startRFID() {
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  USBSerial.println("Prêt à scanner les tags NFC ᕗ");
}


// Lit un tag NFC 
int readRFID() {
  if (rfid.PICC_IsNewCardPresent()){
    USBSerial.println("Une carte est présente !");
    // Verify if the NUID has been readed
    if (rfid.PICC_ReadCardSerial()){
      // Halt PICC and Stop encryption on PCD
      rfid.PICC_HaltA(); rfid.PCD_StopCrypto1();
      // Get card type
      USBSerial.print("\n\n*****************\nCarte type: ");
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      USBSerial.println(rfid.PICC_GetTypeName(piccType));
      // New UID ?
      if (rfid.uid.uidByte[0] != nuidPICC[0] || rfid.uid.uidByte[1] != nuidPICC[1] || rfid.uid.uidByte[2] != nuidPICC[2] || 
        rfid.uid.uidByte[3] != nuidPICC[3] || rfid.uid.uidByte[4] != nuidPICC[4] || rfid.uid.uidByte[5] != nuidPICC[5] || 
        rfid.uid.uidByte[6] != nuidPICC[6] ) {
        // Store NUID into nuidPICC array
        for (byte i = 0; i < 7; i++) { nuidPICC[i] = rfid.uid.uidByte[i]; }
        convHex(rfid.uid.uidByte, rfid.uid.size);
        return(1);
      } else {
        return(2);
      }
    }
  }else {
    return(0);
  }
}


// Converti l'UID en hexa dans newRFID
void convHex(byte *buffer, byte bufferSize) {
  newRFID = "";
  for (byte i = 0; i < bufferSize; i++) {
    char hexStr[3]; // Crée un tableau temporaire pour stocker la valeur hexadécimale
    sprintf(hexStr, "%02X", buffer[i]); // Convertit la valeur en hexadécimal et la stocke dans hexStr
    newRFID += hexStr; // Concatène la valeur hexadécimale à newRFID
    // newRFID += " "; // Ajoute un espace après chaque octet
  }
  // newRFID.trim(); // Supprime les espaces supplémentaires à la fin de la chaîne
}


void procTagLog(){
  USBSerial.println("C'est la procédure procTagLog !");
  // Récupère l'Index de la table log de la DB et l'incrémente
  getIndex(getToDB(apiGetIndexTagLog));
  // Créer le corps de la requête POST
  StaticJsonDocument<1024> reqBody;
  reqBody["Index"] = zIndex;
  reqBody["UID RFID"] = newRFID;
  reqBody["Comment"] = zVERSION;
  reqBody["SSID"] = WiFi.SSID();
  reqBody["RSSI"] = WiFi.RSSI();
  reqBody["IP"] = WiFi.localIP();
  String jsonReqBody;
  serializeJson(reqBody, jsonReqBody);
  // Post la requête à la DB
  postToDB(apiPostNewRecordTagLog, jsonReqBody);
  leds[ledOk] = CRGB::Green; FastLED.show();
  delay(300);
  leds[ledOk] = CRGB::Black; FastLED.show();
}






void clearAllProcedures(){
  USBSerial.println("C'est la procédure clearAllProcedures !");
  zProcAddFromage = false; leds[ledProcAddFromage] = CRGB::Black; FastLED.show();
  zProcAddInventaire = false; leds[ledProcAddInventaire] = CRGB::Black; FastLED.show();
  zProcAddTagCmd = false; leds[ledProcAddTagCmd] = CRGB::Black; FastLED.show();
  zProcNotation = false; leds[ledProcNotation] = CRGB::Black; FastLED.show();
}


void procFromager(){
  USBSerial.println("C'est la procédure procFromager !");
  // A encore faire: initialiser la variable fromager avec le nom du fromager !
  leds[ledProcFromager] = CRGB::Green; FastLED.show();
}


void procAddFromage(){
  USBSerial.println("C'est la procédure procAddFromage !");
  leds[ledProcAddFromage] = CRGB::Yellow; FastLED.show();
  delay(300);
  leds[ledProcAddFromage] = CRGB::Black; FastLED.show();
}


void procAddInventaire(){
  USBSerial.println("C'est la procédure procAddInventaire !");
  leds[ledProcAddInventaire] = CRGB::Green; FastLED.show();
  delay(300);
  leds[ledProcAddInventaire] = CRGB::Black; FastLED.show();
}


void procAddTagCmd(){
  USBSerial.println("C'est la procédure procAddTagCmd !");
  leds[ledProcAddTagCmd] = CRGB::Yellow; FastLED.show();
  delay(300);
  leds[ledProcAddTagCmd] = CRGB::Black; FastLED.show();
}


void procNotation(){
  USBSerial.println("C'est la procédure procNotation !");
  leds[ledProcNotation] = CRGB::Green; FastLED.show();
  delay(300);
  leds[ledProcNotation] = CRGB::Black; FastLED.show();
}



// Traitement du TAG, existe-t-il dans la table tag cmd ?
void toDoTag(){
  String zRequest = apiGetRfidTagCmd; zRequest.replace("xxx", newRFID);
  USBSerial.print("zRequest: "); USBSerial.println(zRequest);
  String payload = getToDB(zRequest);
  USBSerial.print("payload: "); USBSerial.println(payload);

  // Désérialise le JSON
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    USBSerial.print("deserializeJson() failed: "); USBSerial.println(error.f_str());
    leds[ledOk] = CRGB::Red; FastLED.show(); delay(300); leds[ledOk] = CRGB::Black; FastLED.show();
    return;
  }
  // Récupère  le champ "totalRows" pour voir si le tag existe dans la table tag cmd ?
  int zIsCmdTag = doc["pageInfo"]["totalRows"].as<int>();
  USBSerial.print("zIsCmdTag: "); USBSerial.println(zIsCmdTag);

  // C'est un tag cmd
  if(zIsCmdTag == 1){
    zTypeCmd = doc["list"][0]["Type cmd"].as<String>();
    USBSerial.print("C'est le tag cmd: "); USBSerial.println(zTypeCmd);
    itIsTagCmd();

  // Ce n'est pas un tag cmd
  }else if(zIsCmdTag == 0){
    USBSerial.println("Ce n'est pas un tag cmd");
    itIsNotTagCmd();
  
  // y'a un doublon de tag dans la table tag cmd
  }else{
      USBSerial.println("y'a un doublon de tag dans la table tag cmd");
  } 
}


// c'est un tag cmd
void itIsTagCmd(){
  clearAllProcedures();
  if(zTypeCmd == "procFromager"){
    procFromager();
  }
  if(zTypeCmd == "procAddTagCmd"){
    zProcAddTagCmd = true;
    leds[ledProcAddTagCmd] = CRGB::Green; FastLED.show();
  }
}


// ce n'est pas un tag cmd
void itIsNotTagCmd(){
  byte tagUnknow = true;
  if(zProcAddTagCmd){
    procAddTagCmd();
    tagUnknow = false;
  }
  if(zProcNotation){
    procNotation();
    tagUnknow = false;
  }
  if(tagUnknow){
    USBSerial.println("007, on a un problème tag inconnu !");
    leds[ledOk] = CRGB::Red; FastLED.show(); delay(300); leds[ledOk] = CRGB::Black; FastLED.show();
  }
}


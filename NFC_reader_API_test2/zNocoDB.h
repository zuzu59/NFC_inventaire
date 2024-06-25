// zf240625.1732

String zCmdType = "";
String zCmdComment = "";

String zTagFromager = "";
String zTagNotation = "";

bool zProcAddInventaire = false;
bool zProcAddTagCmd = false;
bool zProcNotation = false;

long zStartNumber = 0;


// API JSON
#include <ArduinoJson.h>
const char* zToken = apiToken;
long zIndex = 0;                // Index du record, peut être multiple et doit être généré à la main 
long zId = 0;                   // Id (NocoDB) du record, ne peut être que unique et est auto-généré par

const char* apiGetIndexToto = apiServerName "/api/v2/tables/mccwrj43jwtogvs/records?viewId=vwwm6yz0uhytc9er&fields=Index&sort=-Index&limit=1&shuffle=0&offset=0";
const char* apiPostNewRecordToto = apiServerName "/api/v2/tables/mccwrj43jwtogvs/records";

const char* apiGetIndexTagCmd = apiServerName "/api/v2/tables/mmkk01cafw4ynyp/records?fields=Index&sort=-Index&limit=1&shuffle=0&offset=0";
const char* apiPostNewRecordTagCmd = apiServerName "/api/v2/tables/mmkk01cafw4ynyp/records";
const char* apiPatchTagCmd = apiServerName "/api/v2/tables/mmkk01cafw4ynyp/records";
const char* apiGetRfidTagCmd = apiServerName "/api/v2/tables/mmkk01cafw4ynyp/records?viewId=vw68oujklglmmlp3&where=%28UID%20RFID%2Ceq%2Cxxx%29&limit=25&shuffle=0&offset=0";
const char* apiGetStartNumberCmd = apiServerName "/api/v2/tables/mmkk01cafw4ynyp/records?fields=Comment%2Cid&where=where%3D%28Type%20cmd%2Ceq%2CgetStartNumber%29&limit=25&shuffle=0&offset=0";

const char* apiGetIndexTagLog = apiServerName "/api/v2/tables/md736jl0ppzh1jj/records?viewId=vwl66xl4gwk919f1&fields=Index&sort=-Index&limit=1&shuffle=0&offset=0";
const char* apiPostNewRecordTagLog = apiServerName "/api/v2/tables/md736jl0ppzh1jj/records";

const char* apiPostNewFromageInventaire = apiServerName "/api/v2/tables/m8mwhjo08d8tm72/records";


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


void getStartNumber(){
  String zRequest = apiGetStartNumberCmd;
  USBSerial.print("zRequest: "); USBSerial.println(zRequest);
  String payload = getToDB(zRequest);
  USBSerial.print("payload: "); USBSerial.println(payload);
  // Désérialise le JSON
  DynamicJsonDocument zRecordCmd(1024);
  DeserializationError error = deserializeJson(zRecordCmd, payload);
  if (error) {
    USBSerial.print("deserializeJson() failed: "); USBSerial.println(error.f_str());
    leds[ledOk] = CRGB::Red; FastLED.show(); delay(300); leds[ledOk] = CRGB::Black; FastLED.show();
    return;
  }
  // Récupère  l'id du record"
  zId = zRecordCmd["list"][0]["id"].as<long>();
  USBSerial.print("Id: "); USBSerial.println(zId);
  // Récupère  la valeur de StartNumber dans le champ "Comment"
  zStartNumber = zRecordCmd["list"][0]["Comment"].as<long>();
  USBSerial.print("zStartNumber: "); USBSerial.println(zStartNumber);
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


static void patchToDB(String zApiPatchToDb, String jsonReqBody) {
  // USBSerial.println("PzApiPatchToDb: " + zApiPatchToDb);
  // USBSerial.println("jsonReqBody: " + jsonReqBody);
  // Effectuer la requête PATCH pour modifier un enregistrement
  http.begin(zApiPatchToDb);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("xc-token", zToken);
  int httpCode = http.PATCH(jsonReqBody);
  if (httpCode > 0) {
    USBSerial.printf("PATCH request response code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      String response = http.getString();
      USBSerial.println("PATCH request response:");
      USBSerial.println(response);
    }
  } else {
    USBSerial.println("Error on PATCH request");
  }
  http.end();
}


void postIncStartNumber(){
  USBSerial.println("C'est la procédure postIncStartNumber !");
  // calcul le prochain numéro de fromage
  zStartNumber++;
  // Créer le corps de la requête PATCH
  StaticJsonDocument<1024> reqBody;
  reqBody["id"] = zId;
  reqBody["Comment"] = zStartNumber;
  String jsonReqBody;
  serializeJson(reqBody, jsonReqBody);
  // Patch la requête à la DB
  patchToDB(apiPatchTagCmd, jsonReqBody);
}


void clearAllProcedures(){
  USBSerial.println("C'est la procédure clearAllProcedures !");
  // zProcAddFromage = false; leds[ledProcAddFromage] = CRGB::Black; FastLED.show();
  zProcAddInventaire = false; leds[ledProcAddInventaire] = CRGB::Black; FastLED.show();
  zProcAddTagCmd = false; leds[ledProcAddTagCmd] = CRGB::Black; FastLED.show();
  zProcNotation = false; leds[ledProcNotation] = CRGB::Black; FastLED.show();
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
  reqBody["Fromager"] = zTagFromager;
  String jsonReqBody;
  serializeJson(reqBody, jsonReqBody);
  // Post la requête à la DB
  postToDB(apiPostNewRecordTagLog, jsonReqBody);
  leds[ledOk] = CRGB::Green; FastLED.show();
  delay(300);
  leds[ledOk] = CRGB::Black; FastLED.show();
}


void procFromager(){
  USBSerial.println("C'est la procédure procFromager !");
  zTagFromager = zCmdComment;
  USBSerial.println("C'est le fromager: " + zTagFromager);
  leds[ledProcFromager] = CRGB::Green; FastLED.show();
}


void procAddTagCmd(){
  USBSerial.println("C'est la procédure procAddTagCmd !");
  leds[ledProcAddTagCmd] = CRGB::Green; FastLED.show();
  // Récupère l'Index de la table tag cmd de la DB et l'incrémente
  getIndex(getToDB(apiGetIndexTagCmd));
  // Créer le corps de la requête POST
  StaticJsonDocument<1024> reqBody;
  reqBody["Index"] = zIndex;
  reqBody["UID RFID"] = newRFID;
  String jsonReqBody;
  serializeJson(reqBody, jsonReqBody);
  // Post la requête à la DB
  postToDB(apiPostNewRecordTagCmd, jsonReqBody);
  // Reset le flag add tag cmd
  zProcAddTagCmd = false;
  delay(300);
  USBSerial.println("Tag Cmd ajouté !");
  leds[ledProcAddTagCmd] = CRGB::Black; FastLED.show();
}


void procReboot(){
  USBSerial.println("C'est la procédure procReboot !");
  leds[ledProcNotation] = CRGB::Green; FastLED.show();
  delay(1000);
  ESP.restart();
  leds[ledProcNotation] = CRGB::Black; FastLED.show();
}


void procStop(){
  USBSerial.println("C'est la procédure procStop !");
  leds[ledProcNotation] = CRGB::Green; FastLED.show();
  delay(1000);
  clearAllProcedures();
  leds[ledProcNotation] = CRGB::Black; FastLED.show();
}





void procAddInventaire(){
  USBSerial.println("C'est la procédure procAddInventaire !");
  leds[ledProcAddInventaire] = CRGB::Green; FastLED.show();
  delay(300);

  // Récupère le numéro du fromage à entrer dans l'inventaire depuis le commentaire de getStartNumber
  getStartNumber();

  // Créer le corps de la requête POST
  StaticJsonDocument<1024> reqBody;
  reqBody["Index"] = zStartNumber;
  reqBody["Puce NFC"] = newRFID;
  String jsonReqBody;
  serializeJson(reqBody, jsonReqBody);
  // Post la requête à la DB
  postToDB(apiPostNewFromageInventaire, jsonReqBody);

  // Incrémente le numéro de fromage dans la DB
  postIncStartNumber();

  USBSerial.print("Nouveau fromage ");
  USBSerial.print(zStartNumber);
  USBSerial.println(" ajouté dans l'inventaire !");
  leds[ledProcAddInventaire] = CRGB::Blue; FastLED.show();
}









void procNotation(){
  USBSerial.println("C'est la procédure procNotation !");
  leds[ledProcNotation] = CRGB::Green; FastLED.show();
  delay(300);
  leds[ledProcNotation] = CRGB::Black; FastLED.show();
}


// c'est un tag cmd
void itIsTagCmd(){
  clearAllProcedures();
  if(zCmdType == "procFromager"){
    procFromager();
  }
  if(zCmdType == "procAddInventaire"){
    zProcAddInventaire = true;
    leds[ledProcAddInventaire] = CRGB::Blue; FastLED.show();
    delay(300);
  }
  if(zCmdType == "procAddTagCmd"){
    zProcAddTagCmd = true;
    leds[ledProcAddTagCmd] = CRGB::Yellow; FastLED.show();
  }
  if(zCmdType == "procReboot"){
    procReboot();
  }
  if(zCmdType == "procStop"){
    procStop();
  }
}


// ce n'est pas un tag cmd
void itIsNotTagCmd(){
  byte tagUnknow = true;
  if(zProcAddInventaire){
    procAddInventaire();
    tagUnknow = false;
  }
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


// Traitement du TAG, existe-t-il dans la table tag cmd ?
void toDoTag(){
  String zRequest = apiGetRfidTagCmd; zRequest.replace("xxx", newRFID);
  USBSerial.print("zRequest: "); USBSerial.println(zRequest);
  String payload = getToDB(zRequest);
  USBSerial.print("payload: "); USBSerial.println(payload);

  // Désérialise le JSON
  DynamicJsonDocument zRecordCmd(1024);
  DeserializationError error = deserializeJson(zRecordCmd, payload);
  if (error) {
    USBSerial.print("deserializeJson() failed: "); USBSerial.println(error.f_str());
    leds[ledOk] = CRGB::Red; FastLED.show(); delay(300); leds[ledOk] = CRGB::Black; FastLED.show();
    return;
  }
  // Récupère  le champ "totalRows" pour voir si le tag existe dans la table tag cmd ?
  int zIsCmdTag = zRecordCmd["pageInfo"]["totalRows"].as<int>();
  USBSerial.print("zIsCmdTag: "); USBSerial.println(zIsCmdTag);

  // C'est un tag cmd
  if(zIsCmdTag == 1){
    zCmdType = zRecordCmd["list"][0]["Type cmd"].as<String>();
    zCmdComment = zRecordCmd["list"][0]["Comment"].as<String>();
    USBSerial.println("C'est le tag cmd: " + zCmdType + " avec le commentaire: " + zCmdComment);
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



#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

char* getFlama = "http://192.168.1.89:8080/flama?numFrames=20";
char* getNumFrames = "http://192.168.1.89:8080/numframes";

String response ="";
WiFiClient client;
HTTPClient http;
bool downloaded = false;
#define numFrames 20

int bytes[numFrames][16]={};
byte llamita[numFrames][16]={};

void setup() {
    
    setupWifi();
    updateNumFrames();
}
void updateNumFrames(){
    response = httpGETRequest(getFlama);
}


void setupWifi(){
  Serial.begin(115200);

    WiFi.begin("MiFibra-9E00", "t0rs0t0rs0");
    
    Serial.println("Connecting");
    while(WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    
    
}

void loop() {
    
    if(!downloaded){
      downloadFlama();
    }
}

void downloadFlama(){
    if(WiFi.status()== WL_CONNECTED){
      response = httpGETRequest(getFlama);
      
      //Serial.println(response);
      downloaded = true;

      
      DynamicJsonDocument doc(12288);
      // Parse the JSON input
      DeserializationError err = deserializeJson(doc, response);
      // Parse succeeded?
      if (err) {
        Serial.print(F("deserializeJson() returned "));
        Serial.println(err.f_str());
        return;
      }
      JsonArray by = doc["bytes"];

     
      for (int i = 0; i < by.size(); i++) {
        JsonArray frame = by[i];
        for(int j = 0; j < frame.size(); j++){
          int row = frame[j];
          Serial.print(row);
          llamita[i][j] = row;
        }
        Serial.println();
      }
      //bytes=doc["bytes"];
      
      //Serial.println(by(1));


//      for (int i = 0; i < bytes.length(); i++) {
//        JSONVar value = myObject[keys[i]];
//        Serial.print(keys[i]);
//        Serial.print(" = ");
//        Serial.println(value);
//      }
      
    }
    else {
      Serial.println("WiFi Disconnected");
    }
  
}

String httpGETRequest(const char* serverName) {
  http.begin(serverName);
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  //http.end();

  return payload;
}

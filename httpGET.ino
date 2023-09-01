#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 2

#define CLK_PIN   18 
#define DATA_PIN  17 
#define CS_PIN    16 

#define numFrames 20

MD_MAX72XX matrix = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

int cols = 8;

char* getFlama = "http://192.168.1.89:8080/flama?numFrames=20";
char* getNumFrames = "http://192.168.1.89:8080/numframes";

WiFiClient client;
HTTPClient http;
String response ="";
bool downloaded, parsed = false;

byte llamita[numFrames][16]={};

void setup() {
    
    setupWifi();
    updateNumFrames();

    matrix.begin();
    matrix.clear();
    cols = matrix.getColumnCount();
}
void updateNumFrames(){
  response = httpGETRequest(getFlama);
}


void setupWifi(){
  Serial.begin(115200);
  WiFi.begin("", "");
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500),
    Serial.print(".");
  }
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());    
}

void loop() {
    
  if(!downloaded){
    downloadFlama();
  }

  if(parsed){
    drawFlama();
  }
}

void drawFlama(){
  for ( int j = 0; j < numFrames-1; j++ ) {
    for (int i = 0; i < cols; i++ ) {
        matrix.setColumn(i, llamita[j][cols-1-i]);
    }
    delay(5);
  }  
  delay(5);
  for ( int j = numFrames-2; j >= 0; j-- ) {
    for (int i = 0; i < cols; i++ ) {
        matrix.setColumn(i, llamita[j][cols-1-i]);
    }
    delay(5);
  }  
}

void downloadFlama(){
  if(WiFi.status()== WL_CONNECTED){
    response = httpGETRequest(getFlama);
    downloaded = true;
    parseJsonBody();
  }else{
    Serial.println("WiFi Disconnected");
  }
}

void parseJsonBody(){
  DynamicJsonDocument doc(12288);
  DeserializationError err = deserializeJson(doc, response);
      
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
  parsed = true;
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

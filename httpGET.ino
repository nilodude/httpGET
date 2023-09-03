#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 2

#define CLK_PIN   18 
#define DATA_PIN  17 
#define CS_PIN    16 

#define numFrames 270

MD_MAX72XX matrix = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

int cols = 8;

char* getFlamaURL = "http://192.168.1.89:8080/flama?numFrames=";
char* getNumFramesURL = "http://192.168.1.89:8080/numFrames";

const byte flamaUrlSize =  sizeof getFlamaURL + 3;
char flamaURL[flamaUrlSize];


WiFiClient client;
HTTPClient http;

bool downloaded = false;

byte llamita[numFrames][16]={

};

/*
  delayFrames = 5 ms for 374 frames
  
  delayFrames = 8 ms for 270 frames
 */
unsigned int delayFrames = 8;

void setup() {
    setupWifi();
    updateNumFrames();

    matrix.begin();
    matrix.clear();
    cols = matrix.getColumnCount();
}



void setupWifi(){
  Serial.begin(115200);
  WiFi.begin("MiFibra-9E00", "t0rs0t0rs0");
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500),
    Serial.print(".");
  }
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());    
}

void updateNumFrames(){
  http.begin(getNumFramesURL);
  
  int httpResponseCode = http.GET();
  char response[3];
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Status: ");
    Serial.println(httpResponseCode);
   
    WiFiClient& stream = http.getStream();

    int charIndex = 0;
    while (stream.available()){
      char c = stream.read();
      if(charIndex < 3){
        response[charIndex++] = c;
      }
    }
  }else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  
  strncpy(flamaURL, getFlamaURL, flamaUrlSize);
  strncat(flamaURL, response, flamaUrlSize - strlen(flamaURL));
  Serial.println(flamaURL);
}

void loop() {
    
  if(!downloaded){
    downloadFlama();
  }

  if(downloaded){
    drawFlama();
  }
}

void downloadFlama(){
  if(WiFi.status()== WL_CONNECTED){
    parseHTTPStream();
  }else{
    Serial.println("WiFi Disconnected");
  }
}

void parseHTTPStream() {
  
  http.begin(flamaURL);
  
  int httpResponseCode = http.GET();
    
  if (httpResponseCode>0) {
    Serial.print("HTTP Status: ");
    Serial.println(httpResponseCode);
   
    WiFiClient& stream = http.getStream();
    
    int charNum = 0;
    int frameIndex = 0;
    int rowIndex = 0;
    int charIndex = 0;
    byte row = 0;
    char rowChar[3] = "";
    
    while (stream.available()){
      charNum++;
      char c = stream.read();
      
      if(c != '[' && c != ',' && c !=']'){
         rowChar[charIndex++] = c;
         row = byte(atoi(rowChar));
         llamita[frameIndex][rowIndex] = row;
      }
      
      if(c == ']'){
        frameIndex++;
        charIndex = 0;
        rowChar[0]='\0';
        rowChar[1]='\0';
        rowChar[2]='\0';
      }else if(c== ','){
        rowIndex++;
        charIndex = 0;
        rowChar[0]='\0';
        rowChar[1]='\0';
        rowChar[2]='\0';
      }else if(c == '['){
        rowIndex = 0;
      }
    }
    
    Serial.println();
    Serial.print("parsed ");
    Serial.print(charNum);
    Serial.print(" characters, ");
    Serial.print(--frameIndex);
    Serial.println(" frames");
    
    downloaded = true;
  }else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    downloaded = true;
  }
  
  http.end();
}

void drawFlama(){
  for ( int j = 0; j < numFrames-1; j++ ) {
    for (int i = 0; i < cols; i++ ) {
        matrix.setColumn(i, llamita[j][cols-1-i]);
    }
    delay(delayFrames);
  }  
  delay(delayFrames);
  for ( int j = numFrames-2; j >= 0; j-- ) {
    for (int i = 0; i < cols; i++ ) {
        matrix.setColumn(i, llamita[j][cols-1-i]);
    }
    delay(delayFrames);
  }  
}

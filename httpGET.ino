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

#define numFrames 200

MD_MAX72XX matrix = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

int cols = 8;

char* getFlama = "http://192.168.1.89:8080/flama?numFrames=200";
char* getNumFrames = "http://192.168.1.89:8080/numframes";

WiFiClient client;
HTTPClient http;
char responseC[17000];
String response ="";
bool downloaded, parsed = false;

byte llamita[numFrames][16]={

};

void setup() {
    
    setupWifi();
    updateNumFrames();

    matrix.begin();
    matrix.clear();
    cols = matrix.getColumnCount();
}
void updateNumFrames(){
  //responseFrames = httpGETRequest(getNumFrames);
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

void loop() {
    
  if(!downloaded){
    downloadFlama();
  }

  if(downloaded){
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
  }else{
    Serial.println("WiFi Disconnected");
  }
}

String httpGETRequest(const char* url) {
  
  http.begin(url);
  http.addHeader("Transfer-Encoding", "chunked");
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Status: ");
    Serial.println(httpResponseCode);
   
    WiFiClient& stream = http.getStream();
    Serial.println("printing response stream: ");

    int frameNum = 0;
    int charNum = 0;
    int rowIndex = 0;
    byte row = 0;
    char rowChar[3] = "";
    int charIndex = 0;
    
    while (stream.available()){
      char c = stream.read();
      
      if(c != '[' && c != ',' && c !=']'){
         rowChar[charIndex++] = c;
                  
         row = byte(atoi(rowChar));
         
         uint8_t bitsCount = sizeof( row ) * 8;
         char str[ bitsCount + 1 ];

         uint8_t i = 0;
         while ( bitsCount-- ){
           //str[ i++ ] = bitRead( row, bitsCount ) + '0';
           str[ i++ ] = bitRead( row, bitsCount ) + '0';
         }
         str[ i ] = '\0';
         Serial.println(str);
         
         llamita[frameNum][rowIndex] = row;
         
      }
      
      responseC[charNum] = c;
      charNum++;
      //Serial.print(c);
      if(c == ']'){
        frameNum++;
        charIndex = 0;
        rowChar[0]='\0';
        rowChar[1]='\0';
        rowChar[2]='\0';
        Serial.println();
      }else if(c== ','){
        //Serial.println(rowChar);
        //Serial.println(row,BIN);
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
    Serial.print(--frameNum);
    Serial.println(" frames");
     
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  //http.end();

  return payload;
}

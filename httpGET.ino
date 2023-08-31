#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

const char* serverName = "http://192.168.1.89:8080/flama?numFrames=4";
String response ="";
WiFiClient client;
HTTPClient http;
bool downloaded = false;

void setup() {
    
    Serial.begin(115200);

    WiFi.begin("", "");
    
    Serial.println("Connecting");
    while(WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    
    http.begin(serverName);
}

void loop() {
    
    if(!downloaded){
      downloadFlama();
    }
}

void downloadFlama(){
    if(WiFi.status()== WL_CONNECTED){
      response = httpGETRequest(serverName);
      
      Serial.println(response);
      
      JSONVar myObject = JSON.parse(response);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (response == "{}" || JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
    
      Serial.println(myObject["data"]["bytes"]);

      // myObject.keys() can be used to get an array of all the keys in the object
//      JSONVar keys = myObject.keys();
//    
//      for (int i = 0; i < keys.length(); i++) {
//        JSONVar value = myObject[keys[i]];
//        Serial.print(keys[i]);
//        Serial.print(" = ");
//        Serial.println(value);
//      }
      downloaded = true;
    }
    else {
      Serial.println("WiFi Disconnected");
    }
  
}

String httpGETRequest(const char* serverName) {
  
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

/*
 ESP32 Connect to Wi-Fi
 http:://www.electronicwings.com
*/ 


#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "DIGITALMAGIC-2.4G"; 
const char* password = "DIGITALMAGIC2025!";

void initWiFi() {
 WiFi.mode(WIFI_STA);    //Set Wi-Fi Mode as station
 WiFi.begin(ssid, password);   
 
 Serial.println("Connecting to WiFi ..");
 while (WiFi.status() != WL_CONNECTED) {
   Serial.print('.');
   delay(1000);
 }
 
 Serial.println(WiFi.localIP());
 Serial.print("RRSI: ");
 Serial.println(WiFi.RSSI());
}

void setup() {
 Serial.begin(115200);
 initWiFi();
}

void loop() {
 // put your main code here, to run repeatedly:
}
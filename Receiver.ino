

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <heltec.h> 

#define BAND    451E6  //you can set band here directly,e.g. 868E6,915E6
String rssi = "RSSI --";
String packSize = "--";
String packet ;
String loRaMessage;
String dataLat;
String dataLong;
String dataID;
String dataSym;

const char* ssid = "Elektronika BKM 2";
const char* password = "gelatikbkm2";
String serverName = "http://sanshost.net/api/v1/kirim-data";

unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

void LoRaData(){
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0 , 15 , "Received "+ packSize + " bytes");
  Heltec.display->drawStringMaxWidth(0 , 26 , 128, packet);
  Heltec.display->drawString(0, 0, rssi);  
  Heltec.display->display();
}

void cbk(int packetSize) {
  packet ="";
  packSize = String(packetSize,DEC);
  for (int i = 0; i < packetSize; i++) { packet += (char) LoRa.read(); }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
  LoRaData();
}

void setup() { 
   //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
 
  Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_10);
  delay(1500);
  Heltec.display->clear();
  
  Heltec.display->drawString(0, 0, "Heltec.LoRa Initial success!");
  Heltec.display->drawString(0, 10, "Wait for incoming data...");
  Heltec.display->display();
  delay(1000);
  //LoRa.onReceive(cbk);
  LoRa.receive();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {    // received a packet
    Serial.print("Received packet '");
    while (LoRa.available()) {    // read packet
      packet = LoRa.readString();
      //Serial.print(packet);

      int pos1 = packet.indexOf('/');
      int pos2 = packet.indexOf('&');
      int pos3 = packet.indexOf('&');
      dataID = packet.substring(0, pos1);
      dataSym = packet.substring(pos1 +1, pos2);
      dataLat = packet.substring(pos2+1, +14);
      dataLong = packet.substring(pos2+7, packet.length());
    }
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());   // print RSSI of packet
    Heltec.display->clear();
    Heltec.display->setTextAlignment (TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(0, 0, rssi+String(LoRa.packetRssi())+ "Rx: " + String(packetSize) + " Bytes");
    Heltec.display->drawString(0, 20, packet);
    //Serial.println(packet);
    packet = "";
    Heltec.display->display ();

    Serial.print("dataID: ");
    Serial.println(dataID);
    Serial.print("dataSym: ");
    Serial.println(dataSym);
    Serial.print("dataLat: ");
    Serial.println(dataLat);
    Serial.print("dataLong: ");
    Serial.println(dataLong);

    //millis() - lastTime > timerDelay
    if (LoRa.available()) { 
      StaticJsonDocument<200> doc;
      String LoRaData;
        doc["dataID"] = String(dataID);
        doc["dataSym"] = String(dataSym);
        doc["dataLat"] = String(dataLat);
        doc["dataLong"] = String(dataLong);
  
      WiFiClient client;
      HTTPClient http;
      
      http.begin(client, serverName);
      http.addHeader("Content-Type", "application/json");

      serializeJson(doc, LoRaData);
      Serial.print("POST data >> ");
      Serial.println(LoRaData);
  
      int httpCode = http.POST(LoRaData);//Send the request
      
      String payload;
      if (httpCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpCode);
        String payload = http.getString();
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpCode);
      }
      
      http.end();   //Close connection
      lastTime = millis();
    }
    delay (1000); // wait for a second
  }
}

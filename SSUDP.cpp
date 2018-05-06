
// Send JSON formatted messages
#include <ArduinoJson.h>

// Include WiFi libraries
#include "SSUDP.h"
#include <WiFi101.h>

SSUDP::SSUDP() {}

SSUDP::SSUDP(IPAddress ip, char ssid[], char pass[], int p) {
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) 
  {
    Serial.println("WiFi shield not present");
    while (true);
  }
  // attempt to connect to Wifi network:
  int status;
  while ( status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    WiFi.config(ip);
    status = WiFi.begin(ssid, pass);
  }
  Serial.println("Connected!");
  
  udp.begin(p);
}

void SSUDP::connectDoor(IPAddress ip) {
  door = ip;
}

bool SSUDP::warn(char dn[], int source) {

  StaticJsonBuffer<200> jsonBuffer;

  // Encode the message we're sending into a json object
  JsonObject& message = jsonBuffer.createObject();
  message["status"] = "warn";
  message["method"] = (source == 1) ? "temperature" : "flame";
  message["sender"] = dn;

  // Generate the JSON as a char array
  String json;
  message.printTo(json);
  char * msg = new char [json.length()];
  strcpy(msg, json.c_str());

    Serial.print("Should send ");
  Serial.println(json);

  // Send the message to the door
  return this->send(msg);
}

// (EEET Phone Home....)
bool SSUDP::et(char dn[]) {
  
  StaticJsonBuffer<200> jsonBuffer;

  // Encode the message we're sending into a json object
  JsonObject& message = jsonBuffer.createObject();
  message["status"] = "et";
  message["sender"] = dn;

  // Generate the JSON as a char array
  String json;
  message.printTo(json);
  char * msg = new char [json.length()];
  strcpy(msg, json.c_str());

  // Send the message to the door
  return this->send(msg);
  
}

// Private methods
bool SSUDP::send(char msg[]) {
  udp.beginPacket(door, 8989);
  udp.write(msg);
  udp.endPacket();
}



#include <Time.h>
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

bool SSUDP::warn() {
  return this->send("/status warn");
}

// (EEET Phone Home....)
bool SSUDP::et() {
  char message[] = "/et";
  return this->send(message);
}

// Private methods
bool SSUDP::send(char msg[]) {
  udp.beginPacket(door, 8989);
  udp.write(msg);
  udp.endPacket();
}


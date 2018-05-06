#include <Time.h>
#include <WiFi101.h>
#include <SPI.h>         // needed for Arduino versions later than 0018
#include <WiFiUDP.h>

class SSUDP {

  private:
  
    WiFiUDP udp;
    IPAddress door;

    bool send(char[]);
    
  public:

    SSUDP();
    SSUDP(IPAddress, char[], char[], int);
    void connectDoor(IPAddress);
    
    bool warn();
    bool et(); // ET PHONE HOME
    
};


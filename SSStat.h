#include <WiFi101.h>
#include <SPI.h>         // needed for Arduino versions later than 0018
#include <WiFiUDP.h>

class SSStat {

  private:
  
    double historical[1440];
    int h = 0;
    double current[10];
    int c = 0;

    double threshold;

    /* ********** STAT METHODS ********** */
    
    
  public:

     SSStat(double);

    bool add(double);
    bool isFire(double);

  double t();
    
    double mu();
    double std_dev();
    double xi();
    
};


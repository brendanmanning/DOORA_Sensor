
  /**
   * **************************************************** *
   *                    DOORA (SENSOR)                    *
   *              Bishop Shanahan STEM Team               * 
   *             Programmer: Brendan Manning              *
   *            Copyright 2018 Brendan Manning            *
   * **************************************************** *
   */
   

  // Import required libraries for the high accuracy temperature sensor
  #include <Adafruit_Sensor.h>
  #include <DHT.h>
  #include <DHT_U.h>

  // Define constants used by the temperature sensor
  #define DHTPIN 2
  #define DHTTYPE DHT22
  DHT_Unified dht(DHTPIN, DHTTYPE);

  // Import required WiFi libraries
  #include <WiFi101.h>
  #include <WiFiUdp.h>
  #include "SSUDP.h";

  // Define WiFi Objects
  SSUDP ssudp;
  WiFiUDP Udp;

  // Statistics Stuff
  #include "SSStat.h";
  SSStat stat;


  /**
   * --- Rutime constants ---
   * 
   * iteration: Increments everytime loop() called. Resets to 0 after 10 iterations. Used to limit readings from temperature sensor
   * tests_run: If in testing mode, sets to true after one call of void loop() ... means tests will only be called once
   */
  int iteration = 0;
  bool tests_run = false;
  int led_state = 0;

  /**
   * --- Runtime options ---
   * RUN_TESTS: If set to true, program runs unit tests instead of program itself
   * LOG_MAIN: If true, program outputs logging messages from Main class
   */
  bool RUN_TESTS = true;
  bool LOG_MAIN = true;

void setup() {

  // Initialize the built-in LED
  pinMode(6, OUTPUT);
  
  Serial.begin(9600);

  //ssudp = SSUDP("sunset_home", "Lucy@1226", 8989);

  IPAddress ip(192,168,1,244);
  //ssudp.connectDoor(ip);
  
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  
}

void loop() {  

  // Should we run tests?
  if(RUN_TESTS) {
    do_run_tests();
    return;
  }

  /*// Tell the door we're still here
  if(iteration >= 29) {
    Serial.print("[outgoing]: phoning home...");
    ssudp.et();
    Serial.println("Done.");
    iteration = 0;
  }  

  
  if(stat.isFire(read_temperature())) {
      Serial.println("[outgoing]: warning of fire...");
      ssudp.warn();
      Serial.println("Done.");
    } else {
      Serial.println("[internal]: no fire detected.");
    }

  // 1 iteration == 1 second
  delay(1000);
  iteration++;*/
  
}

double read_temperature() {
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    return -1;
  }
  else {
    return event.temperature;
  }
}

void do_run_tests() {

  bool allTestsPassed = true;  

  stat = SSStat();
  
  stat.isFire(22);
  stat.isFire(23);
  stat.isFire(23.5);
  stat.isFire(23);
  stat.isFire(24);
  stat.isFire(25);
  stat.isFire(24);
  stat.isFire(23.9);
  stat.isFire(120);
  stat.isFire(121);

  /*
   * The average resulting from this data should be 42.94
   */

  if(stat.xi() == 42.94) {
    Serial.println("--    Test XI [✓]   --");
  } else {
    Serial.println("--    Test XI [X]   --0");
    Serial.print("xi() returned: ");
    Serial.println(stat.xi());
    allTestsPassed = false;
  }

  stat.isFire(25);
  stat.isFire(25);
  stat.isFire(26.5);
  stat.isFire(23);
  stat.isFire(24.6);
  stat.isFire(25.1);
  stat.isFire(24.9);
  stat.isFire(26.9);
  stat.isFire(25.6);
  stat.isFire(29.6);


  /**
   * The average of this population should be 25.62
   */

  stat.isFire(27);
  stat.isFire(24);
  stat.isFire(29.5);
  stat.isFire(27);
  stat.isFire(23.6);
  stat.isFire(20.1);
  stat.isFire(27.9);
  stat.isFire(23.6);
  stat.isFire(26.3);
  stat.isFire(23.3);


  /**
   * ********************************************************
   * 
   *              Standard Deviation Test
   * 
   * The average of this population should be 25.23
   * The standard deviation of this population should be 2.64
   * ********************************************************
   */
   
  if(abs(stat.std_dev() - 2.78) <= 0.1) {
    Serial.println("-- Test STD_DEV [✓] --");
   } else {
    Serial.println("-- Test STD_DEV [X] --");
    Serial.print("std_dev() returned: "); Serial.println(stat.std_dev());
    allTestsPassed = false;
   }
  

   /**
    * ************************************
    * 
    *               Mu Test
    * 
    * The resulting x bar should be 31.263
    * 
    * (rounding makes it hard to compare
    *  but make sure they are close)
    * ************************************
    */

    if(abs(31.26 - stat.mu()) <= 0.01) {
      Serial.println("--    Test MU [✓]   --");
    } else {
      Serial.println("--    Test MU [X]   --");
      Serial.print("mu() returned: ");
      Serial.println(stat.mu());
      allTestsPassed = false;
    }


   /**
    * ***********************************
    * 
    *               T Test
    *               
    * The resulting t value should be
    */
    if(abs(stat.t() + 6.854) <= 0.1) {
      Serial.println("--    Test T [✓]    --");
    } else {
      Serial.println("--    Test T [X]    --");
      Serial.print("t() returned: ");
      Serial.println(stat.t());
      allTestsPassed = false;
    }

  
   // Did all tests pass?
   if(allTestsPassed == true) {
    Serial.println("----------------------");
    Serial.println("     All Tests [✓]    ");
    Serial.println("----------------------");
   } else {
    Serial.println("----------------------");
    Serial.println("     All Tests [!]    ");
    Serial.println("  Some tests failed!  ");
    Serial.println("----------------------");
   }
}

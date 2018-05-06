
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

  /**
   * --- Rutime constants ---
   * 
   * iteration: Increments everytime loop() called. Resets to 0 after 10 iterations. Used to limit readings from temperature sensor
   * tests_run: If in testing mode, sets to true after one call of void loop() ... means tests will only be called once
   */
  int iteration = 0;
  bool tests_run = false;

  /**
   * --- Runtime options ---
   * bool RUN_TESTS: If set to true, program runs unit tests instead of program itself
   * bool LOG_MAIN: If true, program outputs logging messages from Main class
   * 
   * int ITERATION_LENGTH: Milliseconds to wait between each iteration
   * int ET_INTERVAL: Device will phone home ever ET_INTERVAL iterations
   * 
   * int TEMP_THRESHOLD: Minimum temperature needed to consider presence of fire (in conjunction with t())
   * double T_THRESHOLD: T value needed to indicate presence of fire (in conjunction with temperature)
   * 
   * IPAddress ip = The IP address the sensor should assign itself to
   * unsigned int LOCAL_PORT: Port to send messages to (should be the same port as on door)
   * 
   * char[] WIFI_NETWORK: WiFi network name to connect to
   * char[] WIFI_PASSWORD: WiFi network password
   * 
   */
  bool RUN_TESTS = false;
  bool LOG_MAIN = true;
  
  int ITERATION_LENGTH = 1000;
  int ET_INTERVAL = 30;
  
  int TEMP_THRESHOLD = 90;
  double T_THRESHOLD = 2.04;

  // Define WiFi Objects
  SSUDP ssudp;
  WiFiUDP Udp;

  // Statistics Stuff
  #include "SSStat.h";
  SSStat stat(T_THRESHOLD);

void setup() {

  // Initialize the built-in LED
  pinMode(6, OUTPUT);

  // Initialize console
  Serial.begin(9600);

  // Connect to WiFi and begin UDP communication
  ssudp = SSUDP(WIFI_NETWORK, WIFI_PASSWORD, 8989);
  IPAddress ip(192,168,1,244);
  ssudp.connectDoor(ip);

  // Begin reading sensor values
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

  // Phone home ever ET_INTERVAL iterations
  if(iteration == ET_INTERVAL - 1) {
    iteration = -1;
    ssudp.et();
  }
  iteration++;
  

  // Check for a fire
  double temperature = read_temperature();
  Serial.print("Temperature: ");
  Serial.println(temperature);
  if(temperature >= TEMP_THRESHOLD) {
    if(stat.isFire(temperature)) {
      ssudp.warn();
      Serial.println("Detected fire!!!");
    }
  }

  // Limit reads to once/second
  delay(ITERATION_LENGTH);

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

  stat = SSStat(T_THRESHOLD); // It doesn't matter the value of T_THRESHOLD. All we're testing here is the math
  
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

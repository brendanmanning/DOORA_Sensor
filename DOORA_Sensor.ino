/**
 * **************************************************** *
                      DOORA (SENSOR)
                Bishop Shanahan STEM Team
               Programmer: Brendan Manning
              Copyright 2018 Brendan Manning
 * **************************************************** *
 */

// Import required libraries for the high accuracy temperature sensor
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// Import required WiFi libraries
#include <WiFi101.h>
#include <WiFiUdp.h>
#include "SSUDP.h";

/**
   --- Rutime constants ---

   iteration: Increments everytime loop() called. Resets to 0 after 10 iterations. Used to limit readings from temperature sensor
   tests_run: If in testing mode, sets to true after one call of void loop() ... means tests will only be called once
*/
int iteration = 0;
bool tests_run = false;

/**
   --- Runtime options ---
   bool RUN_TESTS: If set to true, program runs unit tests instead of program itself
   bool LOG_CONFIG: If set to true, program outputs all runtime options (values in this comment) at the end of void setup()

   int SENSOR_MODE: If 1, temperature ... If 2, flame sensor ... If 3, flame sensor calibration

   int calibration_status_indicator_pin

   double[] calibration_normal_values: EVeryday values for the testing interval
   int[] calibration_flame_values: Values for while we're supplying a flame
   int calibration_index: Index in the calibration array for adding values
   int calibration_mode: 1 = collecting normal values , 2 = collecting flame values , 3 = done

   double calibration_normal_average
   double calibration_flame_average
   doublt calibration_delta_average

   int ITERATION_LENGTH: Milliseconds to wait between each iteration
   int ET_INTERVAL: Device will phone home ever ET_INTERVAL iterations

   int F_THRESHOLD: Max value indicating a fire based on flame
   int TEMP_THRESHOLD: Minimum temperature needed to consider presence of fire (in conjunction with t())
   double T_THRESHOLD: T value needed to indicate presence of fire (in conjunction with temperature)

   int INPUT_PIN: Pin for the flame/temperature sensor

   IPAddress THIS_IP = This IP address the sensor should assign itself to
   IPAddress DOOR_IP = The IP address of the door
   unsigned int REMOTE_PORT: Port to send messages to (should be the same port as on door)

   char[] WIFI_NETWORK: WiFi network name to connect to
   char[] WIFI_PASSWORD: WiFi network password

   char[] DEVICE_NAME: Name of this device. Will be sent to the door with each packet for identification purposes

*/
bool RUN_TESTS = false;
bool LOG_CONFIG = true;

int SENSOR_MODE = 3;

int calibration_status_indicator_pin = 6;

double calibration_normal_values[60];
double calibration_flame_values[620];
int calibration_index = 0;
int calibration_mode = 1;

double calibration_normal_average = -1;
double calibration_flame_average = -1;
double calibration_delta_average = -1;

// Temperature: Sample 1/s .... Flame: Sample 1/ms
int ITERATION_LENGTH = (SENSOR_MODE == 1) ? 1000 : 1000;
int ET_INTERVAL = (SENSOR_MODE == 1) ? 30 : 30;

int F_THRESHOLD = 85;
int TEMP_THRESHOLD = 90;
double T_THRESHOLD = 2.04;

int INPUT_PIN = (SENSOR_MODE == 1) ? 2 : A0;

IPAddress THIS_IP(192, 168, 1, 150);
IPAddress DOOR_IP(192, 168, 1, 244);
unsigned int REMOTE_PORT = 8989;

char WIFI_NETWORK[] = "Alcatel LINKZONE 4212";
char WIFI_PASSWORD[] = "01524212";

char DEVICE_NAME[] = "Tallahassee";

// Define WiFi Objects
SSUDP ssudp;
WiFiUDP Udp;

// Define constants used by the temperature sensor
#define DHTPIN INPUT_PIN
#define DHTTYPE DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);

// Statistics Stuff
#include "SSStat.h";
SSStat stat(T_THRESHOLD);

void setup() {

  // Initialize the built-in LED
  pinMode(calibration_status_indicator_pin, OUTPUT);

  // Initialize console
  Serial.begin(9600);

  // Connect to WiFi and begin UDP communication
  ssudp = SSUDP(THIS_IP, WIFI_NETWORK, WIFI_PASSWORD, REMOTE_PORT);
  ssudp.connectDoor(DOOR_IP);

  delay(1000);
  delay(1000);
  delay(1000);
  delay(1000);

  // Begin reading sensor values
  switch (SENSOR_MODE) {
    case 1: {
        dht.begin();
        sensor_t sensor;
        dht.temperature().getSensor(&sensor);
      }
    case 2: {
        //Serial.println("Initing input pin");
        //pinMode(INPUT_PIN, INPUT);
      }
  }

  // Log config values if requested
  if (LOG_CONFIG) {
    do_log_config();
  }
}

void loop() {

  // Should we run tests?
  if (RUN_TESTS) {
    do_run_tests();
    return;
  }

  // Phone home ever ET_INTERVAL iterations
  if (iteration == ET_INTERVAL - 1) {
    iteration = -1;
    ssudp.et(DEVICE_NAME);
  }
  iteration++;


  // Check for a fire
  switch (SENSOR_MODE) {

    case 1: {
        double temperature = read_temperature();
        Serial.print("Temperature: ");
        Serial.println(temperature);
        if (temperature >= TEMP_THRESHOLD) {
          if (stat.isFire(temperature)) {
            ssudp.warn(DEVICE_NAME, SENSOR_MODE);
            Serial.println("Detected fire [temperature]!!!");
          }
        }
      }
    case 2: {
        Serial.println(analogRead(INPUT_PIN));
        if (analogRead(INPUT_PIN) < F_THRESHOLD) {
          ssudp.warn(DEVICE_NAME, SENSOR_MODE);
          Serial.println("Detected fire [flame]!!!");
        }
      }
    case 3: {
        if (calibration_mode == 1) {
          calibration_normal_values[calibration_index] = analogRead(INPUT_PIN);
          calibration_index++;
          Serial.print("Collected calibration (normal) #"); Serial.print(calibration_index); Serial.print(" ==> "); Serial.println(calibration_normal_values[calibration_index - 1]);
          if (calibration_index == 60) {
            calibration_index = 0;
            calibration_mode = 2;
          }
        } else if (calibration_mode == 2) {
          Serial.println("Reading flame...");
          tone(A5, 1000, 1000);
          calibration_flame_values[calibration_index] = analogRead(INPUT_PIN);
          calibration_index++;
          Serial.print("Collected calibration (flame) #"); Serial.print(calibration_index); Serial.print(" ==> "); Serial.println(calibration_flame_values[calibration_index - 1]);
          if (calibration_index == 60) {
            calibration_index = 0;
            calibration_mode = 3;

            calibration_normal_average = SSStat::average(calibration_normal_values);
            calibration_flame_average = SSStat::average(calibration_flame_values);
            calibration_delta_average = calibration_normal_average - calibration_flame_average;

          }
        } else {
          Serial.println("********** CALIBRATION **********");
          Serial.print("* Ambient light: "); Serial.println(calibration_normal_average);
          Serial.print("* Flame light: "); Serial.println(calibration_flame_average);
          Serial.print("* Delta: "); Serial.println(calibration_delta_average);
          Serial.println("*********************************");

          F_THRESHOLD = calibration_flame_average;

          tone(A5, 1000, 500);
          delay(500);
          tone(A5, 1000, 500);
          delay(500);
          tone(A5, 1000, 500);

          SENSOR_MODE = 2;
        }
      }
  }

  // Limit reads to once every second/millisecond (depending on SENSOR_MODE)
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
     The average resulting from this data should be 42.94
  */

  if (stat.xi() == 42.94) {
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
     The average of this population should be 25.62
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

                  Standard Deviation Test

     The average of this population should be 25.23
     The standard deviation of this population should be 2.64
   * ********************************************************
  */

  if (abs(stat.std_dev() - 2.78) <= 0.1) {
    Serial.println("-- Test STD_DEV [✓] --");
  } else {
    Serial.println("-- Test STD_DEV [X] --");
    Serial.print("std_dev() returned: "); Serial.println(stat.std_dev());
    allTestsPassed = false;
  }


  /**
   * ************************************

                   Mu Test

     The resulting x bar should be 31.263

     (rounding makes it hard to compare
      but make sure they are close)
   * ************************************
  */

  if (abs(31.26 - stat.mu()) <= 0.01) {
    Serial.println("--    Test MU [✓]   --");
  } else {
    Serial.println("--    Test MU [X]   --");
    Serial.print("mu() returned: ");
    Serial.println(stat.mu());
    allTestsPassed = false;
  }


  /**
   * ***********************************

                   T Test

     The resulting t value should be
  */
  if (abs(stat.t() + 6.854) <= 0.1) {
    Serial.println("--    Test T [✓]    --");
  } else {
    Serial.println("--    Test T [X]    --");
    Serial.print("t() returned: ");
    Serial.println(stat.t());
    allTestsPassed = false;
  }


  // Did all tests pass?
  if (allTestsPassed == true) {
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

void do_log_config() {
  Serial.println("******************** RUNTIME CONFIGURATION *********************");
  Serial.print(" * RUN_TESTS: "); Serial.println(RUN_TESTS);
  Serial.print(" * LOG_CONFIG: "); Serial.println(LOG_CONFIG);
  Serial.print(" * SENSOR_MODE: "); Serial.println(SENSOR_MODE);
  Serial.print(" * ITERATION_LENGTH: "); Serial.println(ITERATION_LENGTH);
  Serial.print(" * ET_INTERVAL: "); Serial.println(ET_INTERVAL);
  Serial.print(" * TEMP_THRESHOLD: "); Serial.println(TEMP_THRESHOLD);
  Serial.print(" * T_THRESHOLD: "); Serial.println(T_THRESHOLD);
  Serial.print(" * INPUT_PIN: "); Serial.println(INPUT_PIN);
  Serial.print(" * THIS_IP: "); Serial.println(THIS_IP);
  Serial.print(" * DOOR_IP: "); Serial.println(DOOR_IP);
  Serial.print(" * REMOTE_PORT: "); Serial.println(REMOTE_PORT);
  Serial.print(" * WIFI_NETWORK: "); Serial.println(WIFI_NETWORK);
  Serial.print(" * WIFI_PASSWORD: "); Serial.println(WIFI_PASSWORD);
  Serial.print(" * DEVICE_NAME: "); Serial.println(DEVICE_NAME);
  Serial.println("****************************************************************");
}

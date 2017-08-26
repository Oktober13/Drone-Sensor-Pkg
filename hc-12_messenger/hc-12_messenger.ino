// The code for the Arduino Nano and sensor package, which is transmitting data on height and wind speed/direction back to the Arduino Uno

#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <LIDARLite.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// SENSORS

// Sparkfun GARMIN LIDAR-LITE V3

LIDARLite myLidarLite;

// BAROMETER - ADAFRUIT BMP280

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11 
#define BMP_CS 10
Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

// WIND SENSOR - MODERN DEVICE REV. P

const int OutPin1 = A0; // 6 wind sensor analog pin hooked up to Wind P sensor “OUT” pin
const int TempPin1 = A1; // 6 temp sesnsor analog pin hooked up to Wind P sensor “TMP” pin
const int OutPin2 = A2; // 6 wind sensor analog pin hooked up to Wind P sensor “OUT” pin
const int TempPin2 = A3; // 6 temp sesnsor analog pin hooked up to Wind P sensor “TMP” pin
const int OutPin3 = A6; // 6 wind sensor analog pin hooked up to Wind P sensor “OUT” pin
const int TempPin3 = A7; // 6 temp sesnsor analog pin hooked up to Wind P sensor “TMP” pin

float Vw1 = 1.406;                            //Voltage due to wind
float Vt1;                                    //Voltage due to temperature
float TempC1 = 0.000000;                      //
float Vw2 = 1.406;                            //
float Vt2;                                    //
float TempC2 = 0.000000;                      //
float Vw3 = 1.406;                            //
float Vt3;                                    //
float TempC3 = 0.000000;                      //

float ZeroWindV = 1.406;                      //

// TRANSMISSION CYCLE VARIABLES

#define transCycle 500U                     // Transmission cycle
unsigned long LastMillis = 0;
int transwhich = 0;                              // Send temp = 0, send wind = 1, send wind direction = 2

// DATA MEMORY

float currAlt = 0.00;   // Current height in m; from LIDAR
float altSum = 0.00;    // Sum of 20 altitude readings from LIDAR
char Alt[7];            // Buffer big enough for 6-character float

float lastTemp = 0.00;
float currTemp = 0.00;
char temp[7]; // Buffer big enough for 6-character float

float currWind1 = 0.00; // Current wind in mph; Sensor 1
float currWind2 = 0.00; // Current wind in mph, Sensor 2
float currWind3 = 0.00; // Current wind in mph, Sensor 3
float currWind = 0.00;  // Current wind overall, curr. determined by strongest reading
float lastWind = 0.00;
char wind[7]; // Buffer big enough for 6-character float
char dir[3];

// TRANSMISSION CODE

const byte HC12RxdPin = 6;                  // Recieve Pin on HC12
const byte HC12TxdPin = 5;                  // Transmit Pin on HC12

SoftwareSerial HC12(HC12TxdPin,HC12RxdPin); // Create Software Serial Port

void setup() {
  Serial.begin(9600);                       // Open serial port to computer
  HC12.begin(9600);                         // Open serial port to HC12

  myLidarLite.begin(0, true); // Set configuration to default and I2C to 400 kHz
  myLidarLite.configure(0); // Change this number to try out alternate configurations
 /*
    configure(int configuration, char lidarliteAddress)

    Selects one of several preset configurations.

    Parameters
    ----------------------------------------------------------------------------
    configuration:  Default 0.
      0: Default mode, balanced performance.
      1: Short range, high speed. Uses 0x1d maximum acquisition count.
      2: Default range, higher speed short range. Turns on quick termination
          detection for faster measurements at short range (with decreased
          accuracy)
      3: Maximum range. Uses 0xff maximum acquisition count.
      4: High sensitivity detection. Overrides default valid measurement detection
          algorithm, and uses a threshold value for high sensitivity and noise.
      5: Low sensitivity detection. Overrides default valid measurement detection
          algorithm, and uses a threshold value for low sensitivity and noise.
    lidarliteAddress: Default 0x62. Fill in new address here if changed. See
      operating manual for instructions.
  */
  
  if (!bmp.begin()) {  
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }
}

void loop() {
  //if(HC12.available()){                     // If HC-12 Rx buffer has data
  //  Serial.write(HC12.read());              // Send the data to the Arduino
  //  }
  
  if (cycleCheck(&LastMillis, transCycle)){
    if (transwhich == 0){
//      sendNewTemp();
      sendNewAlt();
      transwhich++;
    }
    else if (transwhich == 1){
      sendNewWind();
      transwhich++;
    }
    else{
      sendNewDir();
      transwhich = 0;
    }
  }
  
  if(Serial.available()){   // If data has been sent and HC-12 has reception
    char data = Serial.read();
    //char data = bmp.readTemperature();
    //if (data != '\n'){
    HC12.write(data);                // Send that data to HC-12 for transmission
    //}
  }
}

boolean cycleCheck(unsigned long *lastMillis, unsigned int cycle) 
{
 unsigned long currentMillis = millis();
 if(currentMillis - *lastMillis >= cycle)
 {
   *lastMillis = currentMillis;
   return true;
 }
 else
   return false;
}

// TRANSMISSION FUNCTIONS

void sendNewTemp(){
  currTemp =  bmp.readTemperature();
  currTemp = (currTemp * 1.8) + 32;
  if (lastTemp != currTemp){
    dtostrf(currTemp, 4, 2, temp);          // Leave room for too large numbers!
    temp[5] = 'T';
//    Serial.println(temp);
    HC12.write(temp);
  }
  
  lastTemp = currTemp;  
}

void sendNewAlt(){
  for(int iter = 0; iter <= 10; iter++){
    altSum += myLidarLite.distance();
  }
  currAlt = (altSum * 3.2808) / (100.0 * 10.00);
  dtostrf(currAlt, 4, 2, Alt);          // Leave room for too large numbers!

  for(int index = 0; index < (sizeof(wind) / sizeof(Alt[0])); index += 1){
    if(Alt[index] == NULL){
      Alt[index] = 'A';
      break;
    }
  }
  
//  Alt[5] = 'A';
  Serial.println(Alt);
  HC12.write(Alt);
  altSum = 0;
}

void sendNewDir(){
  dir[1] = 'D';
  Serial.println(dir);
  HC12.write(dir);
}

void sendNewWind(){
  Vw1 = ((float)analogRead(OutPin1) * 5.0) / 1023.0;    //Raw voltage data from wind pin
  Vw2 = ((float)analogRead(OutPin2) * 5.0) / 1023.0;    //Raw voltage data from wind pin
  Vw3 = ((float)analogRead(OutPin3) * 5.0) / 1023.0;    //Raw voltage data from wind pin
//  Serial.print(Vw2);
//  Serial.print("V, ");
  
  Vt1 = ((float)analogRead(TempPin1) * 5.0) / 1023.0;   //Raw voltage data from temp pin
  Vt2 = ((float)analogRead(TempPin2) * 5.0) / 1023.0;   //Raw voltage data from temp pin
  Vt3 = ((float)analogRead(TempPin3) * 5.0) / 1023.0;   //Raw voltage data from temp pin
//  Serial.print(Vt2);
//  Serial.println("V");
  
  TempC1 = ((Vt1 - 0.400)/0.0195);                    //Conversion from volts to C.
  TempC2 = ((Vt2 - 0.400)/0.0195);                    //Conversion from volts to C.
  TempC3 = ((Vt3 - 0.400)/0.0195);                    //Conversion from volts to C.
//TempC =  bmp.readTemperature();
//  Serial.print(TempC2);
//  Serial.print("C, ");
  
  currWind1 = pow((((Vw1-1.406) / (3.038517 * pow(TempC1,0.115157) )) / 0.087288),  3.009364);
  currWind2 = pow((((Vw2-1.406) / (3.038517 * pow(TempC2,0.115157) )) / 0.087288),  3.009364);
  currWind3 = pow((((Vw3-1.406) / (3.038517 * pow(TempC3,0.115157) )) / 0.087288),  3.009364);
  Serial.print(currWind1);
  Serial.print(", ");
  Serial.print(currWind2);
  Serial.print(", ");
  Serial.println(currWind3);

///////////////////////////// UNCOMMENT THIS ////////////////////////////////////////

//// Direction is measured via an 8-point compass rose- N == 0, NE == 1, E == 2, ..., NW == 7
//  if(((currWind1 + 0.1) > currWind3) && (currWind1 < (currWind3 + 0.1)) && (currWind1 > (currWind2 + 0.1))){ // Direction 0 (N)
  if(((currWind1 + 0.1) > currWind3) && (currWind1 < (currWind3 + 0.1))){ // DELETE THIS
    currWind = (currWind1 + currWind3)/2;
    dir[0] = '0';
  }
//  else if((currWind1 > currWind2 + 0.1) && (currWind1 > currWind3 + 0.1)){ // Direction 1 (E)
  else if((currWind1 > currWind3 + 0.1)){ //// DELETE THIS
    currWind = currWind1;
    dir[0] = '2';
  }
  else if(((currWind1 + 0.1) > currWind2) && (currWind1 < (currWind2 + 0.1)) && (currWind1 > (currWind3 + 0.1))){ // Direction 3 (SE)
    currWind = (currWind1 + currWind2)/2;
    dir[0] = '3';
  }
  else if((currWind2 > (currWind1 + 0.1)) && (currWind2  > (currWind3 + 0.1))){ // Direction 4 (S)
    currWind = currWind2;
    dir[0] = '4';
  }
  else if(((currWind2 + 0.1) > currWind3) && (currWind2 < (currWind3 + 0.1)) && (currWind2 > (currWind1 + 0.1))){ // Direction 5 (SW)
    currWind = (currWind2 + currWind3)/2;
    dir[0] = '0';
  }
//  else if((currWind3 > (currWind1 + 0.1)) && (currWind3  > (currWind2 + 0.1))){ // Direction 6 (W)
  else if((currWind3 > currWind1 + 0.1)){ //// DELETE THIS
    currWind = currWind3;
    dir[0] = '6';
  }
  else{
    currWind = lastWind;
    dir[0] = ' ';
    Serial.println("No wind sensor readings");
  }

  if (lastWind != currWind){
      dtostrf(currWind, 4, 2, wind);          // Leave room for too large numbers!
      for(int index = 0; index < (sizeof(wind) / sizeof(wind[0])); index += 1){
        if(wind[index] == NULL){
          wind[index] = 'W';
          break;
        }
      }
//      wind[4] = 'W';

      Serial.println(wind);
      HC12.write(wind);
  }

  lastWind = currWind;
}

#include <string.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield(); // Init LCD shield

// Defining backlight colors
#define GREEN 0x2
#define WHITE 0x7

//Variables for conversion byte-to-str for LCD display of serial input
char data = '\n';                           // The current byte being received
char databuffer[16] = {NULL};               // Buffer where data is kept while incompletely transferred
int datacount = 0;
boolean clearscreen = false;
boolean startup = true;

// Data storage of old values

char oldAlt[16] = {NULL};
char oldWind[16] = {NULL};
char oldDir[16] = {NULL};

int windDir;

// Custom arrow symbols for display
byte N[8] = {B00100,B01110,B10101,B00100,B00100,B00100,B00000,};
byte NE[8] = {B00000,B01111,B00011,B00101,B01001,B10000,B00000,};
byte E[8] = {B00000,B00100,B00010,B11111,B00010,B00100,B00000,};
byte Se[8] = {B00000,B10000,B01001,B00101,B00011,B01111,B00000,};
byte S[8] = {B00000,B00100,B00100,B00100,B10101,B01110,B00100,};
byte SW[8] = {B00000,B00000,B00001,B10010,B10100,B11000,B11110,};
byte W[8] = {B00000,B00100,B01000,B11111,B01000,B00100,B00000,};
byte NW[8] = {B00000,B11110,B11000,B10100,B10010,B00001,B00000,};
int arrowcount = 0;

// Transmission code
const byte HC12RxdPin = 6;                  // Recieve Pin on HC12
const byte HC12TxdPin = 5;                  // Transmit Pin on HC12

SoftwareSerial HC12(HC12TxdPin,HC12RxdPin); // Create Software Serial Port

// Declaring functions
void ClearScreen();

void setup() {
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.createChar(0, N);
  lcd.createChar(1, NE);
  lcd.createChar(2, E);
  lcd.createChar(3, Se);
  lcd.createChar(4, S);
  lcd.createChar(5, SW);
  lcd.createChar(6, W);
  lcd.createChar(7, NW);

  // Print a message to the LCD.
  lcd.print("Loading...");
  lcd.setCursor(0,0);
  lcd.setBacklight(WHITE);

  // Initialize Serial comms
  Serial.begin(9600);                       // Open serial port to computer
  HC12.begin(9600);                         // Open serial port to HC12
}

void loop() {
  uint8_t buttons = lcd.readButtons();

  if (buttons) {
    lcd.clear();
    lcd.setCursor(0,0);
    if (buttons & BUTTON_UP) {
      lcd.setBacklight(WHITE);
      lcd.print("Loading...");
    }
    if (buttons & BUTTON_DOWN) {
      lcd.setBacklight(GREEN);
      lcd.print("Loading...");
    }
  }


  // For processing transmissions
  if(HC12.available()){                     // If HC-12 has received a transmission
    data = HC12.read();                     // Store in a temporary buffer
    if (data != '\n' && data != 'A' && data != 'W' && data != 'D'){       // If transmission is not finished
      ClearScreen();                        // Check if screen needs to be refreshed
      databuffer[datacount] = data;
      datacount += 1;
      //lcd.write(data);                    // Write data to LCD screen
    }
    
    else if(data == 'A'){                   // If the data sent is temperature data
      Serial.println("Altitude (ft) received!");
      if (databuffer[0] != NULL){
        memcpy(oldAlt, databuffer, sizeof(oldAlt));
      }
      
      lcd.setCursor(5,0);
      for(int index = 0; index < 16; index += 1){
        if(databuffer[index] != NULL){
          lcd.write(databuffer[index]);     // Write data to LCD screen
        }
      }
      
      // Rewriting old data for wind to lcd
      lcd.setCursor(6,1);
      for(int index = 0; index < (sizeof(oldWind) / sizeof(oldWind[0])); index += 1){
        if(oldWind[index] != NULL){
          lcd.write(oldWind[index]);     // Write data to LCD screen
        }
      }
      memset(databuffer, NULL, sizeof databuffer);
      clearscreen = true;                   // New transmission, trigger flag to clear the screen
    }
    
    else if(data == 'W'){                   // If the data sent is wind data
      Serial.println("Wind strength received!");
      if (databuffer[0] != NULL){
        memcpy(oldWind, databuffer, sizeof(oldWind));
      }
      
      lcd.setCursor(6,1);
      for(int index = 0; index < 16; index += 1){
        if(databuffer[index] != NULL){
          lcd.write(databuffer[index]);     // Write data to LCD screen
        }
      }

      // Rewriting old data for alt to lcd
      lcd.setCursor(5,0);
      for(int index = 0; index < (sizeof(oldAlt) / sizeof(oldAlt[0])); index += 1){
        if(oldAlt[index] != NULL){
          lcd.write(oldAlt[index]);     // Write data to LCD screen
        }
      }
      
      memset(databuffer, NULL, sizeof databuffer);
      clearscreen = true;                   // New transmission, trigger flag to clear the screen
    }
    
    else if(data == 'D'){                   // If the data sent is wind direction data
      Serial.println("Wind direction received!");
      if (databuffer[0] != NULL){
        memcpy(oldDir, databuffer, sizeof(oldDir));
      }
      sscanf(oldDir,"%d",&windDir);   // Convert transmitted direction information from a char array into a float

      // Rewriting old data for alt and wind to lcd
      lcd.setCursor(5,0);
      for(int index = 0; index < (sizeof(oldAlt) / sizeof(oldAlt[0])); index += 1){
        if(oldAlt[index] != NULL){
            lcd.write(oldAlt[index]);
        }
      }
      lcd.setCursor(6,1);
      for(int index = 0; index < (sizeof(oldWind) / sizeof(oldWind[0])); index += 1){
        if(oldWind[index] != NULL){
          lcd.write(oldWind[index]);     // Write data to LCD screen
        }
      }
      
      memset(databuffer, NULL, sizeof databuffer);
      clearscreen = true;                   // New transmission, trigger flag to clear the screen
    }
    
    else{
      clearscreen = true;                   // New transmission, trigger flag to clear the screen
    }
  }

  // For troubleshooting the LCD screen
  //if(Serial.available()){                   // If Arduino's computer Rx buffer has a message
  //  char data = Serial.read();              // Store data in temporary buffer
  //  Serial.print(data);                     // Output message to Serial monitor
  //}
}

void ClearScreen(){
  if (clearscreen == true or startup == true){
        lcd.clear();
        
        lcd.setCursor(0,0);
        lcd.print("Alt:       ft");
        
        lcd.setCursor(0,1);
        lcd.print("Wind:       mph");
        
        lcd.setCursor(15,0);
        lcd.write(byte(windDir));
//        lcd.write(byte(arrowcount));
//        
//        arrowcount+=1;
//        if(arrowcount >= 8){
//          arrowcount = 0;
//        }
        //lcd.setCursor(6,0);
        
        databuffer[16] = {NULL};
        clearscreen = false;
        startup = false;
   }
}

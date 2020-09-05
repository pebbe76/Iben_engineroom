#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// Global variables
int sensorPort = 0; //Water level port side  
int sensorStarboard = 0; //Water level starboard side
float sensorTemperatureC = 0; //Air temperature in enine room in centigrade 
float sensorHumidity = 0; //Air hunidity in engine room
String message = "";
int sensorPyro = LOW; // Default no movement on pyro- and movementsensor 
float maxValueStarboard = 705; //Maximum water level, starboard side
float maxValuePort = 715; //Maximum water level, port side
float minValueStarboard = 100;  //Minimum water level, starboard side
float minValuePort = 100; //Minimum water level, port side
float gasLevel = 0; //Level og gas in the air
float tempLog[9]; //Temperature log used for storing last sensor readings to calcuate the speed of change in temp
int gasPresent = LOW; //Default no gas present in engine room
int percentPort = 0; //Water level port side in percent 
int percentStarboard = 0; //Water level starboard side in percent
int displayAlternator = 0; //Controller "bit" for alternating display screen

//OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     LED_BUILTIN // Reset pin

DHT dht(7, DHT22); //Digital pin 7
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  // put your setup code here, to run once:
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x7B for 128x64 (8bit). Remove LIB for = 0x3C for 7bit Arduino max address space
    //Catch error on Display object start
    raiseAlarm();
  }
  
  Wire.begin();
  Serial.begin(9600);
  dht.begin();

  pinMode(2, OUTPUT); // Set buzzer as an output
  
  display.display();
  delay(2000);
  // Clear the buffer
  display.clearDisplay();
}

void loop() {
  // put your main code here, to run repeatedly:

  //reset sensor values
  sensorPort = 0;
  sensorStarboard = 0;
  sensorTemperatureC = 0;
  sensorHumidity = 0;
  sensorPyro = LOW;
  gasLevel = 0;
  gasPresent = LOW;

  delay(3000);

  //read sensor values
  sensorPort = analogRead(A1); 
  sensorStarboard = analogRead(A3); 
  sensorTemperatureC = dht.readTemperature(); //DHT22
  sensorHumidity = dht.readHumidity();
  sensorPyro = digitalRead(6);
  if (digitalRead(4) != LOW){
    gasLevel = analogRead(A5);
  }

  //output values to OLED display
  displayOLED();
  calibrateSensors();
}

void displayOLED (){
  
  display.clearDisplay();
  
  display.setTextSize(1);            
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0); 
  display.print("M/Y Iben engine room");

  display.setTextSize(2);
  switch (displayAlternator) {
    case 1:
      displayAlternator = 2;
      
      display.setCursor(0, 22); 
      //display.print("Temperatur:   ");
      display.print(sensorTemperatureC);
      display.print("C");
      
      display.setCursor(0, 50); 
      //display.print("Luftfuktighet:");
      display.print(sensorHumidity);
      display.print("%");
      
      break;
    case 2:
      displayAlternator = 3;
      
      display.setCursor(0, 22);
      //display.print("Vann S/B:     ");
      display.print(sensorStarboard);
      display.print("/");
      display.print(sensorPort);
      display.display();
      
      display.setCursor(0, 50);
      //display.print("Vann S/B:     ");
      display.print(percentStarboard);
      display.print("%/");
      display.print(percentPort);
      display.print("%");
      break;
    case 3:
      displayAlternator = 4;
      
      display.setCursor(0, 22); 
      display.print("Gasser: ");

      display.setCursor(0, 50); 
      display.print(gasLevel);
      break;
    case 4:
      displayAlternator = 1;

      //display.drawFastHLine(0, 20, 50, SSD1306_WHITE);
      //display.drawFastVLine(0, 20, 50, SSD1306_WHITE);

      //const uint8_t PROGMEM level[] = {

      display.setCursor(0, 22); 

      //int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color
      //drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
      display.drawRect(0,17,40,44,SSD1306_WHITE); 
      display.drawRect(70,17,40,44,SSD1306_WHITE);

      //fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
      if (percentPort > 10 && percentPort < 25){
        display.fillRect(0,17,40,12,SSD1306_WHITE); 
      }
      else if (percentPort > 25 && percentPort < 50){
        display.fillRect(0,17,40,22,SSD1306_WHITE); 
      }
      else if (percentPort > 50 && percentPort < 75){
        display.fillRect(0,17,40,33,SSD1306_WHITE); 
      }
      else if (percentPort > 75){
        display.fillRect(0,17,40,44,SSD1306_WHITE); 
      }

      break;
    default:
      displayAlternator = 1;
      break;
  }
    if ((sensorPyro != LOW) || (gasLevel > 200)) {
    raiseAlarm();
  }

  display.display();
}

void raiseAlarm(){
   //Visual alarm

   display.clearDisplay();
   display.setTextSize(4);
   display.setCursor(0,4);
   display.print("ALARM");
  
  //Sonic alarm - Digital pin 2
  tone(2, 1000); // Send 1KHz sound signal...
  delay(1000);   // Tone will keep sounding for 1 sec
  noTone(2);     // Stop sound...
  tone(2, 1500); // Send 1,5KHz sound signal...
  delay(1000);   // Tone will keep sounding for 1 sec
  noTone(2);     // Stop sound...
  tone(2, 2000); // Send 2KHz sound signal...
  delay(1000);   // Tone will keep sounding for 1 sec
  noTone(2);     // Stop sound...

}

void calibrateSensors(){
  //Calibrate port side water sensors

  if (sensorPort > maxValuePort){
    maxValuePort = sensorPort;
  }   
  else if ((sensorPort < minValuePort) || (minValuePort == 0)  ){
    minValuePort = sensorPort;
  }
  //Calibrate starboard side water sensors
  if (sensorStarboard > maxValueStarboard){
    maxValueStarboard = sensorStarboard;
  }
  else if ((sensorStarboard < minValueStarboard) || (minValueStarboard == 0)){
    minValueStarboard = sensorStarboard;
  }

  percentPort = ((sensorPort-minValuePort)/(maxValuePort-minValuePort) * 100);
  percentStarboard = ((sensorStarboard-minValueStarboard)/(maxValueStarboard-minValueStarboard)*100);
  sensorTemperatureC = 0;
  //sensorTemperatureF = 0;
  sensorHumidity = 0;
 
}

float calculateTempChange(){
  //Calculate the speed of change in engine room temperature. Monitors the last 10 temperature sensor reading and returns the change in percent
  //after QA; remove use of 10 log entries - only need first and last value (reduce memory allocation and performance)

  float changePercent = 0;
  
  for (int i = 0; i <= 9; i++){
    
    if (i == 9){
      tempLog[i] = sensorTemperatureC;
    }
    else {
      tempLog [i] = tempLog [i+1];
    }
    //tempLog[i] = sensorTemperatureC;
  }
  if (tempLog [0] != NULL && tempLog[9] != NULL){
    changePercent = tempLog[9] / tempLog [0];
  }
  return changePercent;
 
}

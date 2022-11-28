///////////////////////////  NOTES  ///////////////////////////////////////////
/* Serial.println() commands have been commented out to conserve dynamic memory   
 *    *Avoids stability problems
 * 
 * Code modified from Gyawali et al. "Talking SMAAC" 
 * 
 * I2C addresses of sensors changed by modifying the configuration of the interface board
*/
///////////////////////////////////////////////////////////////////////////////




#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <NDIR_I2C.h>
#include "RTClib.h"
#include "rtc_sync.h"

NDIR_I2C sensor2cm(0x4D); //Adaptor's8 I2C address (7-bit)
NDIR_I2C sensor8cm(0x48); //Three sensors used at varying depths
NDIR_I2C sensor16cm(0x49); 
const int chipSelect = 10; //SD card pin

unsigned long previousMillis = 0;         // see https://www.arduino.cc/en/Tutorial/BlinkWithoutDelay for more

// constants won't change :
const long read_interval = 900000;             //Delay 15 min (900,000 milliseconds) between readings
const long setup_interval = 180000;        // 3 minutes (180,000 milliseconds) is the sensor warm up/stabilization period (See datasheet - "Preheat time")


File logfile;
char filename[] = "CO2_LOG.CSV";          // filename for data file on SD card

// converts DateTime object and returns a string with proper formatting
//      EX: "2019-01-31 17:05:01"
String prettyDate(DateTime dt)
{

    String dataString = "";
    dataString += String(dt.year());
    dataString += "-";
    dataString += String(dt.month());
    dataString += "-";
    dataString += String(dt.day());
    dataString += " ";
    if (dt.hour()<10)
    {
      dataString += "0";
    }
    dataString += String(dt.hour());
    dataString += ":";
    if (dt.minute()<10)
    {
      dataString += "0";
    }
    dataString += String(dt.minute());
    dataString += ":";
   if (dt.second()<10)
    {
      dataString += "0";
    }
    dataString += String(dt.second());

    return dataString;
}

void setup()
{
  // enable serial console
  Serial.begin(9600);

  // set up I/O pins
  pinMode(10, OUTPUT);

  //Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect))
  {
    //Serial.println("Card failed, or not present");
    while (1)
    {
     // Serial.println("SD Card Error");
      delay(5000);
    }
  }
  //Serial.println("card initialized.");
  
  // if the file is available, write to it:
  logfile = SD.open(filename, FILE_WRITE);
  if (logfile)
  {
    //Serial.println("File OK!");
    logfile.close();
  }
  else
  {
   // Serial.println("Error: Missing File");
  }
  //Serial.println("card initialized.");
  
  // initialize the RTC chip
  // if the RTC cannot be initialized, halt
  //Serial.println("Initializing RTC");
  if (!rtc.begin()) {
    //Serial.println("Couldn't find RTC");
    while (1);
  }

  //Serial.println("RTC started");
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //Sets RTC to date and time this sketch was compiled
                                                  //__DATE__ and __TIME__ fetches information from computer
  
  if (!rtc.initialized()) {
    //Serial.println("RTC is NOT running!");
  }


  //Serial.println("Attempting to Initialize sensors");
  if (sensor2cm.begin())
  {
    //Serial.println("2cm Sensor Initialized");
    
    // disable autocalibration, see readme file for more information
    //Serial.println("Disabling Autocalibration");
    sensor2cm.disableAutoCalibration();
  }
  else      
  {
    //Serial.println("ERROR: Failed to connect to the 2cm sensor.");
  }
  
  //Initialize 8cm sensor
  if(sensor8cm.begin())
  {
    //Serial.println("8cm Sensor Initialized");
    sensor8cm.disableAutoCalibration();
  }
  else
  {
     //Serial.println("ERROR: Failed to connect to the 8cm sensor.");
  }


  //Initialize 16cm sensor
  if(sensor16cm.begin())
  {
    //Serial.println("16cm Sensor Initialized");
    sensor16cm.disableAutoCalibration();
  }
  else
  {
     //Serial.println("ERROR: Failed to connect to the 16cm sensor.");
  }

  //Serial.println("Wait 3 minutes for sensor warm-up...");
  
  //Serial.println("Sensor warming up");
  
  previousMillis = millis();

  // wait 10 seconds for the sensor to warm up
  while (true)
  {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= setup_interval)
    {
      break;
    }
      // during the setup period, process any rtc syncronization messages
      syncRTC();
  }
  //Serial.println("Setup complete");
}



void loop()
{
  
  // prepare the message for logging
  // format is <DATE>,<CO2 Concentration 1>,<CO2 Concentration 2>,<CO2 Concentration 3>
  
  String disp = "";

  // get the current timestamp
  DateTime now = rtc.now();

  disp += prettyDate(now);
  disp += ",";
  if (sensor2cm.measure()) 
  {
    disp += sensor2cm.ppm;                                     
  }
  else 
  {
    disp += "null"; //Indicates that sensor failed to measure
  }

  disp += ",";
  
  if (sensor8cm.measure()) 
  {
    disp += sensor8cm.ppm;                                        
  }
  else 
  {
    disp += "null";
  }

  disp += ",";
  if (sensor16cm.measure()) 
  {
    disp += sensor16cm.ppm;                                         
  }
  else 
  {
    disp += "null";
  }


  //Serial.println(disp);

  logfile = SD.open(filename, FILE_WRITE);
  logfile.println(disp);
  logfile.close();

  // wait for the next sensor read time
  previousMillis = millis();
  while (true)
  {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= read_interval)
    {
      break;
    }
    syncRTC();
  }

}

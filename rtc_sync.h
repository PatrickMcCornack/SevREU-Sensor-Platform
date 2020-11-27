/* This file allows easy adjustment to the RTC time on the datalogger shield. This is not required for the system to work. 
 *  A sync message is sent from the Arduino serial interface in the form of "Txxxxxxxxxx" where the x's are replaced with the current unix timestamp.
 *    EX: T1548957540 would set the time to 01/31/2019 @ 5:59pm (UTC)
 *  
 */

RTC_PCF8523 rtc;
char TIME_HEADER = 'T';

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1451606400; // Jan 1 2016 00:00:00.000
  const unsigned long MAX_TIME = 2713910400; // Jan 1 2056 00:00:00.000

  if (Serial.find(TIME_HEADER)) {
    pctime = Serial.parseInt();
    Serial.println("Received:" + String(pctime));
    if ( pctime < DEFAULT_TIME) // check the value is a valid time (greater than Jan 1 2016)
    {
      Serial.println("Time out of range");
      pctime = 0L; // return 0 to indicate that the time is not valid
    }
    if ( pctime > MAX_TIME) // check the value is a valid time (greater than Jan 1 2016)
    {
      Serial.println("Time out of range");
      pctime = 0L; // return 0 to indicate that the time is not valid
    }
  }
  return pctime;
}
void syncRTC()
{
  uint32_t newTs = processSyncMessage();

  if (newTs > 0)
  {
    
    // while not implemented in this version, the time can be shifted to make the RTC time match the local timezone offset by adding the timezone offset to newTs as appropriate
    // Get the old time stamp and print out difference in times
    uint32_t oldTs = rtc.now().unixtime();
    int32_t diffTs = newTs - oldTs;
    int32_t diffTs_abs = abs(diffTs);
    Serial.println("RTC is Off by " + String(diffTs_abs) + " seconds");

    //Display old and new time stamps
    Serial.print("Updating RTC, old = " + String(oldTs));
    Serial.println(" new = " + String(newTs));

    //Update the rtc
    rtc.adjust(newTs);
  }
}

#include <Servo.h>

int leftLightSens = A0; 
int rightLightSens = A1;
int leftLightLvl = 0;
int rightLightLvl = 0;
int threshold = 250;         //threshold light diff before move will be completed
int nightThreshold = 150;    //threshold before declaring night time
long nightStartTime = 0;    //Timestamp night started
long rtnSleepTime = 30000;  //30 second wait before returning home at night
int baseLight = 0;          //baseline light level

Servo myservo;             // create servo object to control a servo
int servoMin = 0;         // minimum servo position
int servoMax = 90;        // maximum servo position
int pos = servoMax;        // current servo position, initalize to the home position
bool reachedStop = false;  // already on a stop?
bool verbosedebug = false; //verbose debugging?
 
void setup() {
  Serial.begin(9600);
  pinMode(leftLightSens, INPUT);
  pinMode(rightLightSens, INPUT);

  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  myservo.write(pos); //return to homeposition
   
  baseLight = (analogRead(leftLightSens)+analogRead(rightLightSens))/2;
  Serial.print("Baseline light level = ");
  Serial.println(baseLight);
}

void loop() {
  int lightDelta = 0;
  leftLightLvl = analogRead(leftLightSens);
  rightLightLvl = analogRead(rightLightSens);
  if (verbosedebug)
  {
    Serial.print("left light= ");
    Serial.print(leftLightLvl);
    
    Serial.print(" right light= ");
    Serial.println(rightLightLvl);
  }
  lightDelta = compareLightLvl();

  if (lightDelta > 0 and pos < servoMax )
  {

    reachedStop = false;   // Moving off the stop if already on one
    pos++;             // move towards higher light by 1 degree 
    if (verbosedebug)
    {
      Serial.print("moving to position ");
      Serial.println(pos);
    }    
    myservo.write(pos);    // tell servo to go to position in variable 'pos'
    delay(15);             // waits 15ms for the servo to reach the position
  }
  else if (lightDelta > 0 and pos == servoMax )
  {
    if (reachedStop == false)
    {
      Serial.println("**** Reached Positive End Stop ****");
      reachedStop = true;
    }
    if (verbosedebug)
    {
      Serial.print("request to move but on positive end stop. left = ");
      Serial.print(leftLightLvl);
      Serial.print(" right= ");
      Serial.println(rightLightLvl);
    }
    
  }
  else if (lightDelta < 0 and pos > servoMin )
  {
    reachedStop = false;   // Moving off the stop if already on one
    pos--;             // move towards higher light by 1 degree       
    if (verbosedebug)
    {
    Serial.print("moving to position ");
    Serial.println(pos);
    }  
    myservo.write(pos);    // tell servo to go to position in variable 'pos'
    delay(15);             // waits 15ms for the servo to reach the position
  }
  else if (lightDelta < 0 and pos == servoMin )
  {
    if (reachedStop == false)
    {
     Serial.println("**** Reached Negative End Stop ****");
     reachedStop = true;
    }
    if (verbosedebug)
    {
      Serial.print("request to move but on negative end stop. left = ");
      Serial.print(leftLightLvl);
      Serial.print(" right= ");
      Serial.println(rightLightLvl);
    }
  }
  else
  {
    //no movement needed. Check if it's night time and we should return to home position
    if (verbosedebug)
    {
    Serial.println("Light Delta = 0, checking if night time");
    }
    if (nightIsScary() && (pos != servoMax))
    {
     //night time return to home
     Serial.println("**** Screw you guys I am going home ****");
     while (pos < servoMax)
     {
        //slowly return home 1 degree every 15 msec
        pos++;
        if (verbosedebug)
        {
        Serial.print("moving to position ");
        Serial.println(pos);
        }   
        myservo.write(pos);
        delay(15);             
     }
    }
  }
 // delay(250);
}

int compareLightLvl(){
  int delta=0;
  delta = rightLightLvl - leftLightLvl;
  if (verbosedebug)
  {
    Serial.print("Raw Delta= ");
    Serial.println(delta);
  }
  
  if (abs(delta) > threshold)
  {
    return delta;
  }
  else
    return 0;
}

bool nightIsScary(){
  long currentMillis = millis();

  if (verbosedebug)
  {
    Serial.print("current time= ");
    Serial.print(currentMillis);
  }
  //average light on each side is below the night time threshold beyond the startup light level.
  if ((((leftLightLvl+rightLightLvl)/2)-baseLight) <= nightThreshold)
  {
    //can't sleep clown will eat me, start timer before going home.
    //sleep to make sure its night not a cloud.
    if (nightStartTime == 0)
    {
      nightStartTime = millis();
      if (verbosedebug)
      {
        Serial.print(" time night started= ");
        Serial.print(nightStartTime);
        Serial.print(" wait before going home= ");
        Serial.print(rtnSleepTime);
      }
    }
    //check if we have waited long enough if so return that it is night time.
    if ((currentMillis - nightStartTime) > rtnSleepTime)
    {
      if (verbosedebug)
      {
        Serial.println(" time to head home!");
      }
      //reset the start of night for next day.
      nightStartTime = 0;  
      return true;
    }
    if (verbosedebug)
      Serial.println(" not yet time to head home.");
  }
  else
   {
    //light level above night time so reset our night wait timer
    //must have been a cloud.
      nightStartTime = 0;
      if (verbosedebug)
        Serial.println(" still daylight!");
   }
   return false;
}

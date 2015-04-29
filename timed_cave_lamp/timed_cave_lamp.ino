/*
 * Code for replacing the firmware on a NANJG LED driver board
 * v0.1 - Sam Wenham
 *
 *
 */

#include <avr/wdt.h>

//declare pins ATTiny13
byte LEDPin = 1;
byte switchPin = 0;
byte battPin = 1;

//declare pins Uno
/*byte LEDPin = 3;
byte switchPin = 2;
byte battPin = A0;*/

byte count = 0;
byte lastmode;
byte mode = 1;
byte waspressed = 0;

int battRead;
int battLowLvl = 175;
byte battLow = 0;
byte battOverride = 0;
byte battLowCnt = 0;

//change modes here; just change/add/delete values. The "0" is 'off' 

byte modes[] = {0,20,50,120,255};          //PWM values, 5..255 - LEAVE THE "0" THERE

void setup() {
  
  pinMode(LEDPin, OUTPUT);
  pinMode(switchPin, INPUT);
  pinMode(battPin, INPUT);
  digitalWrite(switchPin, HIGH); //set pullup
  wdt_enable(WDTO_8S); //8 second watchdog just incase we get stuck
  //Serial.begin(9600);
  
}

ISR(WDT_vect){
  for (byte i=0; i<10; i++){
    analogWrite(LEDPin, modes[0]);
    delay(500);
    analogWrite(LEDPin, modes[2]);
  }
  analogWrite(LEDPin, modes[1]); //Uhoh how did we get here! best turn the lamp on
}

void loop() {
  
  byte switchRead = digitalRead(switchPin);
  if ((switchRead)==0) {                   //when the button is pressed (PB3 pulled down)
      count++;                             //count length of button press
      if (count==80) {                    //pressed long (80*25ms)
        if (mode>0) {
          lastmode=mode; mode=0;           //was on?  -> off, but remember the mode
        }else{
          mode=lastmode;                //was off? -> on, with previous mode.
          if (battOverride==1){         //if we were overriding battmon
            battOverride=0;            //stop doing it
            battLowCnt=0;
          }
        }
      }
      waspressed=1;                        //remember that the button was pressed, see below
    }
    else {                                 //button not pressed
      if (waspressed) {                    //but it was pressed, so it has just been released!
        waspressed=0;                      //reset that
        if (count<80 && mode>0) {          //really was a short press AND the light is on
          if (battLow==1 && battOverride==0){
            battOverride=1; //we have been poked in low batt mode so override
            mode++; if (mode>=sizeof(modes)) mode=1; //next mode
          }else{
            mode++; if (mode>=sizeof(modes)) mode=1; //next mode
          }
        }
        count=0;                           //reset counter
      }
    }
    
  battRead = analogRead(battPin);

  if (battRead < battLowLvl){
    battLow = 1;
    battLowCnt++;   
    if (battOverride == 0 && mode>1 && battLowCnt>80){ //if we haven't overridden and have been stable bump down
      mode--;
      battLowCnt=0;
    }  
  } else if (battRead > (battLowLvl + 10)){
    battLow = 0;
    battLowCnt=0;  //reset the lowbatt counter if the reading is ok again;
    battOverride=0;
  }

  /*
  Serial.print(battRead, DEC);
  Serial.print("-> Override: ");
  Serial.print(battOverride, DEC);
  Serial.write(10); 
  */ 

  analogWrite(LEDPin, modes[mode]);  //set PWM level (0 is off)
  wdt_reset();                       // We wrote something to the PWM so we might be ok!!
  delay(25);                         //wait a bit before checking again, important for counting
}

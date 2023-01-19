/*This code is for the Arduino MEGA - Main Microcontroller
 Written by J. Widjaja and J. A. Hauger at Augusta University in January 2022.

 Program Description: Receives user input to select and control rotarod speed; 
 integrates infrared sensors, stepper motor, LCD panels, LEDs, push buttons, and toggles/power switches. 
*/
//LCD Display Set Up
#include <LiquidCrystal.h>  //standard liquid crystal library
LiquidCrystal lcd(4, 5, 6, 7, 8, 9); //connects LCD to pins 4-9 on Arduino MEGA

//RGB LED indicator pin connections
const byte redLED0 = 34; //RGB LED indicator Chamber 0 pin connection
const byte grnLED0 = 36;
const byte bluLED0 = 37;
const byte redLED1 = 38; //RGB LED indicator Chamber 1 pin connection
const byte grnLED1 = 40;
const byte bluLED1 = 41;
const byte redLED2 = 42; //RGB LED indicator Chamber 2 pin connection
const byte grnLED2 = 44; 
const byte bluLED2 = 45;
const byte redLED3 = 46; //RGB LED indicator Chamber 3 pin connection
const byte grnLED3 = 48; 
const byte bluLED3 = 49;
const byte redLED4 = 50; //RGB LED indicator Chamber 4 pin connections
const byte grnLED4 = 52;
const byte bluLED4 = 53;

//Stepper Motor Pin Connections
const byte dirPin = 26; //Controls stepper motor direction
const byte pulPin = 28; //Controls stepper motor electrical pulses 

//Interrupt Pins linked to Arduino UNO (linked to infrared sensors)
const byte interruptPin0 = 3;      //Signals fallen mouse in chamber 0
const byte interruptPin1 = 18;     //Signals fallen mouse in chamber 1
const byte interruptPin2 = 19;     //Signals fallen mouse in chamber 2
const byte interruptPin3 = 20;     //Signals fallen mouse in chamber 3
const byte interruptPin4 = 21;     //Signals fallen mouse in chamber 4

//User Pushbutton Connections 
const byte startButtonPin = 24;    //Pin connection to start pushbutton (begins mode)
const byte cycleButtonPin = 22;    //Pin connection to cycle through and select mode via pushbutton
const byte killButtonPin = 2;      //Pin connection to emergency stop pushbutton
const byte radiusPin = 23;         //Pin connection to toggle switch to adjust speeds for variable rod diameters

//Initialized Variables
//Timing and State Variables
unsigned long startTime = 0;             //clock reading at beginning of test          
bool timingStarted = 0;                  //logical state variable representing testing has begun
unsigned long endTime0 = 0;              //clock reading when mouse N falls (N = 0, 1, 2, 3, 4)
unsigned long endTime1 = 0;             
unsigned long endTime2 = 0;
unsigned long endTime3 = 0;
unsigned long endTime4 = 0;
unsigned long currentElapsedTime = 0;    //elapsed time since mode begins (millis() - startTime)
float elapsedTime0 = 0;                  //elapsedTimeN = endTimeN - startTime (N = 0, 1, 2, 3, 4)
float elapsedTime1 = 0;
float elapsedTime2 = 0;
float elapsedTime3 = 0;
float elapsedTime4 = 0;

//Pushbutton Variables
int mode = 0;                            //selected mode
bool previousState = 0;                  //state variable for cycle pushbutton
bool startButtonState = 0;               //state variable for start button
volatile byte killButtonState = LOW;     //state variable to override normal operations and end mode

//Stepper Motor Variables
float delayTMillis = 0;                  //Stepper motor pulse delay in milliseconds
float delayTMicros = 0;                  //Stepper motor pulse delay in microseconds
                  
//Mouse Detection Variables
volatile byte mouse0Fell = LOW;          //interrupt state variables for mouse fall detection
volatile byte mouse1Fell = LOW;
volatile byte mouse2Fell = LOW;
volatile byte mouse3Fell = LOW;
volatile byte mouse4Fell = LOW;
bool mouse0Done = 0;                     //state variable to indicate end of test 
bool mouse1Done = 0;
bool mouse2Done = 0;
bool mouse3Done = 0;
bool mouse4Done = 0; 

//Radius Adjustment Variables
float radiusFactor = 0;                  //variable used to represent ratio of actual rod radius to default radius 
bool radiusSwitchState = 0;              //state variable for radius adjustment toggle switch


//Interrupt Service Routines
void mouse0()                            
{
  mouse0Fell = HIGH;                     //Sets state variable to high after mouse fell (interrupt received from Arduino UNO)
}

void mouse1()
{
  mouse1Fell = HIGH;
}
void mouse2()
{
  mouse2Fell = HIGH;
}
void mouse3()
{
  mouse3Fell = HIGH;
}
void mouse4()
{
  mouse4Fell = HIGH;
}
void killButton()                        //Sets state variable to high after kill button is pushed
{
  killButtonState = HIGH;
}


void setup() 
{
  pinMode(pulPin,OUTPUT);       //Stepper Motor Pins
  pinMode(dirPin,OUTPUT);
  pinMode(redLED0,OUTPUT);      //RBG LED Output Pins
  pinMode(grnLED0,OUTPUT); 
  pinMode(bluLED0,OUTPUT);  
  pinMode(redLED1,OUTPUT);
  pinMode(grnLED1,OUTPUT);
  pinMode(bluLED1,OUTPUT);
  pinMode(redLED2,OUTPUT);
  pinMode(grnLED2,OUTPUT);
  pinMode(bluLED2,OUTPUT); 
  pinMode(redLED3,OUTPUT);
  pinMode(grnLED3,OUTPUT);
  pinMode(bluLED3,OUTPUT);
  pinMode(redLED4,OUTPUT);     
  pinMode(grnLED4,OUTPUT);  
  pinMode(bluLED4,OUTPUT);
  pinMode(startButtonPin,INPUT); //Start Button Pin
  pinMode(cycleButtonPin,INPUT); //Mode Selection Pin
  pinMode(radiusPin, INPUT);     //Radius Selection Toggle Switch

  //interrupt pinModes
  pinMode(killButtonPin,INPUT);  //Interrupt from kill button
  pinMode(interruptPin0,INPUT);  //Interrupt received Arduino UNO
  pinMode(interruptPin1,INPUT);
  pinMode(interruptPin2,INPUT);
  pinMode(interruptPin3,INPUT);
  pinMode(interruptPin4,INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin0), mouse0, RISING);      //Assign interrupt service routines     
  attachInterrupt(digitalPinToInterrupt(interruptPin1), mouse1, RISING);
  attachInterrupt(digitalPinToInterrupt(interruptPin2), mouse2, RISING);
  attachInterrupt(digitalPinToInterrupt(interruptPin3), mouse3, RISING); 
  attachInterrupt(digitalPinToInterrupt(interruptPin4), mouse4, RISING);
  attachInterrupt(digitalPinToInterrupt(killButtonPin), killButton, RISING);

  //Print initial messages on LCD
  lcd.begin(20,4);  
  lcd.setCursor(0,0);
  lcd.print("Push Cycle Button");
  lcd.setCursor(0,1);
  lcd.print("to set mode"); 

  //Power Up light sequence (optional)
  digitalWrite(redLED0, LOW);
  digitalWrite(grnLED0, LOW);
  digitalWrite(bluLED0, LOW);
  digitalWrite(redLED1, LOW);
  digitalWrite(grnLED1, LOW);
  digitalWrite(bluLED1, LOW);
  digitalWrite(redLED2, LOW);
  digitalWrite(grnLED2, LOW);
  digitalWrite(bluLED2, LOW);
  digitalWrite(redLED3, LOW);
  digitalWrite(grnLED3, LOW);
  digitalWrite(bluLED3, LOW);
  digitalWrite(redLED4, LOW);
  digitalWrite(grnLED4, LOW);
  digitalWrite(bluLED4, LOW);
  delay(150);
  digitalWrite(redLED0,LOW);
  digitalWrite(grnLED0,HIGH);
  digitalWrite(bluLED0,HIGH);
  digitalWrite(redLED4,LOW);
  digitalWrite(grnLED4,HIGH);
  digitalWrite(bluLED4,HIGH);
  delay(200);
  digitalWrite(redLED1,LOW);
  digitalWrite(grnLED1,HIGH);
  digitalWrite(bluLED1,HIGH);
  digitalWrite(redLED3,LOW);
  digitalWrite(grnLED3,HIGH);
  digitalWrite(bluLED3,HIGH);
  digitalWrite(redLED0,LOW);
  digitalWrite(grnLED0,LOW);
  digitalWrite(bluLED0,LOW);
  digitalWrite(redLED4,LOW);
  digitalWrite(grnLED4,LOW);
  digitalWrite(bluLED4,LOW);
  delay(200);
  digitalWrite(redLED2,LOW);
  digitalWrite(grnLED2,HIGH);
  digitalWrite(bluLED2,HIGH);
  digitalWrite(redLED1,LOW);
  digitalWrite(grnLED1,LOW);
  digitalWrite(bluLED1,LOW);
  digitalWrite(redLED3,LOW);
  digitalWrite(grnLED3,LOW);
  digitalWrite(bluLED3,LOW);
  delay(200);
  digitalWrite(redLED1,LOW);
  digitalWrite(grnLED1,HIGH);
  digitalWrite(bluLED1,HIGH);
  digitalWrite(redLED3,LOW);
  digitalWrite(grnLED3,HIGH);
  digitalWrite(bluLED3,HIGH);
  digitalWrite(redLED2,LOW);
  digitalWrite(grnLED2,LOW);
  digitalWrite(bluLED2,LOW);
  delay(200);
  digitalWrite(redLED0,LOW);
  digitalWrite(grnLED0,HIGH);
  digitalWrite(bluLED0,HIGH);
  digitalWrite(redLED4,LOW);
  digitalWrite(grnLED4,HIGH);
  digitalWrite(bluLED4,HIGH);
  delay(200);
  digitalWrite(redLED1,LOW);
  digitalWrite(grnLED1,HIGH);
  digitalWrite(bluLED1,HIGH);
  digitalWrite(redLED3,LOW);
  digitalWrite(grnLED3,HIGH);
  digitalWrite(bluLED3,HIGH);
  digitalWrite(redLED0,LOW);
  digitalWrite(grnLED0,LOW);
  digitalWrite(bluLED0,LOW);
  digitalWrite(redLED4,LOW);
  digitalWrite(grnLED4,LOW);
  digitalWrite(bluLED4,LOW);
  delay(200);
  digitalWrite(redLED2,LOW);
  digitalWrite(grnLED2,HIGH);
  digitalWrite(bluLED2,HIGH);
  digitalWrite(redLED1,LOW);
  digitalWrite(grnLED1,LOW);
  digitalWrite(bluLED1,LOW);
  digitalWrite(redLED3,LOW);
  digitalWrite(grnLED3,LOW);
  digitalWrite(bluLED3,LOW);
  delay(200);
  digitalWrite(redLED1,LOW);
  digitalWrite(grnLED1,HIGH);
  digitalWrite(bluLED1,HIGH);
  digitalWrite(redLED3,LOW);
  digitalWrite(grnLED3,HIGH);
  digitalWrite(bluLED3,HIGH);
  digitalWrite(redLED2,LOW);
  digitalWrite(grnLED2,LOW);
  digitalWrite(bluLED2,LOW);
  delay(200);
  digitalWrite(redLED0,LOW);
  digitalWrite(grnLED0,HIGH);
  digitalWrite(bluLED0,HIGH);
  digitalWrite(redLED4,LOW);
  digitalWrite(grnLED4,HIGH);
  digitalWrite(bluLED4,HIGH);
  digitalWrite(redLED1,LOW);
  digitalWrite(grnLED1,LOW);
  digitalWrite(bluLED1,LOW);
  digitalWrite(redLED3,LOW);
  digitalWrite(grnLED3,LOW);
  digitalWrite(bluLED3,LOW);
  delay(200);
  digitalWrite(redLED0,LOW);
  digitalWrite(grnLED0,HIGH);
  digitalWrite(bluLED0,HIGH);
  digitalWrite(redLED4,LOW);
  digitalWrite(grnLED4,HIGH);
  digitalWrite(bluLED4,HIGH);
  digitalWrite(redLED1,LOW);
  digitalWrite(grnLED1,HIGH);
  digitalWrite(bluLED1,HIGH);
  digitalWrite(redLED3,LOW);
  digitalWrite(grnLED3,HIGH);
  digitalWrite(bluLED3,HIGH);
  digitalWrite(redLED0,LOW);
  digitalWrite(grnLED0,LOW);
  digitalWrite(bluLED0,LOW);
  digitalWrite(redLED4,LOW);
  digitalWrite(grnLED4,LOW);
  digitalWrite(bluLED4,LOW);
  delay(200);
  digitalWrite(redLED2,LOW);
  digitalWrite(grnLED2,HIGH);
  digitalWrite(bluLED2,HIGH);
  digitalWrite(redLED1,LOW);
  digitalWrite(grnLED1,LOW);
  digitalWrite(bluLED1,LOW);
  digitalWrite(redLED3,LOW);
  digitalWrite(grnLED3,LOW);
  digitalWrite(bluLED3,LOW);
  delay(200);
  digitalWrite(redLED1,LOW);
  digitalWrite(grnLED1,HIGH);
  digitalWrite(bluLED1,HIGH);
  digitalWrite(redLED3,LOW);
  digitalWrite(grnLED3,HIGH);
  digitalWrite(bluLED3,HIGH);
  delay(200);
  digitalWrite(redLED0,LOW);
  digitalWrite(grnLED0,HIGH);
  digitalWrite(bluLED0,HIGH);
  digitalWrite(redLED4,LOW);
  digitalWrite(grnLED4,HIGH);
  digitalWrite(bluLED4,HIGH);
  delay(500);
  digitalWrite(redLED4,LOW);
  digitalWrite(grnLED4,LOW);
  digitalWrite(bluLED4,LOW);
  delay(425);
  digitalWrite(redLED3,LOW);
  digitalWrite(grnLED3,LOW);
  digitalWrite(bluLED3,LOW);
  delay(400);
  digitalWrite(redLED2,LOW);
  digitalWrite(grnLED2,LOW);
  digitalWrite(grnLED2,LOW);
  delay(370);
  digitalWrite(redLED1,LOW);
  digitalWrite(grnLED1,LOW);
  digitalWrite(bluLED1,LOW);
  delay(320);
  digitalWrite(redLED0,LOW);
  digitalWrite(grnLED0,LOW);
  digitalWrite(bluLED0,LOW);
  //end of start up sequence (optional - can be deleted)
} //end of void setup()

void loop() 
{
  if (timingStarted==0)                                       //before start button is pushed
  mode=0;
  {
    while (startButtonState==0) 
    {  
        elapsedTime0=0;                                       //Resets elapsedTime (please do not delete) 
        elapsedTime1=0;
        elapsedTime2=0;
        elapsedTime3=0;
        elapsedTime4=0;
        int loopCount = 0;
        radiusSwitchState = digitalRead(radiusPin);
        if (radiusSwitchState==1)
        {
           radiusFactor = 0.4375; 
        }
        else 
        {
           radiusFactor = 1.0;
        }
        startButtonState = digitalRead(startButtonPin);
        delay(25);
        bool cycleButtonState = digitalRead(cycleButtonPin);
        delay(25);    
        if (cycleButtonState != previousState)
        {
          if (cycleButtonState==1)                                      //cycle button was pressed
          {
            if(loopCount==0)                                            
            { 
                lcd.clear();                                            //Display initializing message once during device power up
                lcd.setCursor(0,0);                                   
                lcd.print("Select Mode");
                lcd.setCursor(0,3);
                lcd.print("Press Start to Begin");
                loopCount=1;                                            //sets state variable to display initializing message only once during device power up
            }
            previousState=1;
            if (mode<0)                                                 //This program contains mode 1-6         
            {
                mode=0;  
            }
            mode++;                                                    //increment mode for each cycle button push
            if (mode==7)                                               //returns to mode 1 after it reaches the last mode                                   
            { 
                mode=1;
            } 
            lcd.setCursor(12,0);
            lcd.print("= ");
            lcd.setCursor(14,0);
            lcd.print(mode);                                           //Display mode number
            switch (mode)                                             
            {
              case 1:
              {
                lcd.setCursor(0,1);
                lcd.print("4rpm for 3 min");                           //Displays mode description on LCD
                break;
              }
              case 2:
              {
                lcd.setCursor(0,1);
                lcd.print("10rpm for 3 min");
                break;
              }
              case 3:
              {
                lcd.setCursor(0,1);
                lcd.print("4-60rpm in 300s");
                break;
              }
              case 4:
              {
                lcd.setCursor(0,1);
                lcd.print("4-60rpm in 150s");
                break;
              }
              case 5:
              {
                lcd.setCursor(0,1);
                lcd.print("4-60rpm in 300s cw");
                break;
              }
              case 6:
              {
                lcd.setCursor(0,1);
                lcd.print("Sequential Program");
                break;
              }
             } //end of switch
           } //end of if (cycleButtonState==1)
         else
         {
             previousState=0;
         }
      } //end of if (cycleButtonState != previousState)
     } 
  }
  if  ((startButtonState==1))                               //Once startbutton is pushed and stepper motor begins based on mode selection
  {
    killButtonState = LOW;                                  //initialize to begin testing
    mouse0Fell=LOW;
    mouse1Fell=LOW;
    mouse2Fell=LOW;
    mouse3Fell=LOW;
    mouse4Fell=LOW;
    switch (mode)
    {
      //********************************************CASE 1: 4rpm for 3 min - ccw**************************************************
        case 1:
        {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Timing...");             //Display reads "Timing. . . " to user
            digitalWrite(redLED0, LOW);         //Sets RGB LEDs to white
            digitalWrite(redLED1, LOW);
            digitalWrite(redLED2, LOW);
            digitalWrite(redLED3, LOW);
            digitalWrite(redLED4, LOW);
            digitalWrite(grnLED0, LOW);
            digitalWrite(grnLED1, LOW);
            digitalWrite(grnLED2, LOW);
            digitalWrite(grnLED3, LOW);
            digitalWrite(grnLED4, LOW);
            digitalWrite(bluLED0, LOW);
            digitalWrite(bluLED1, LOW);
            digitalWrite(bluLED2, LOW);
            digitalWrite(bluLED3, LOW);
            digitalWrite(bluLED4, LOW);
            delay(100);
            startTime = millis();                //read clock
            timingStarted=1;
            while ((millis()-startTime < 180000) && (killButtonState==LOW) && ((mouse0Fell==LOW) || (mouse1Fell==LOW) || (mouse2Fell==LOW) || (mouse3Fell==LOW) || (mouse4Fell==LOW)))
            {                                                   //continue testing for 180000 milliseconds (3 minutes) if kill button has not been pushed and all mice have not fallen 
              digitalWrite(pulPin, HIGH);
              delay (5.074/radiusFactor);                       //delayT in seconds = (60)/(1600*desired RPM)
              digitalWrite (pulPin, LOW);                       //^^calculated based on ratio of bottum pulley (D - 0.846 inch) to the top pulley (D - 1.564 inch); Based on ratio, desired RPM of bottom pulley was 7.39 RPM for 4 RPM rod rotation
              delay (5.074/radiusFactor);
              if ((mouse0Done==0) && (mouse0Fell==HIGH))        
              {
                digitalWrite(redLED0, LOW);                     //Updates RGB LED to red
                digitalWrite(grnLED0, HIGH);
                digitalWrite(bluLED0, HIGH);
                endTime0=millis();
                elapsedTime0 = (endTime0-startTime)/1000.0;     //Record elapsed time after mouse falls
                mouse0Done=1;                                   //sets state variable indicating mouse 0 has fallen
              }
              if ((mouse1Done==0) && (mouse1Fell==HIGH))        
              {
                digitalWrite(redLED1, LOW);
                digitalWrite(grnLED1, HIGH);
                digitalWrite(bluLED1, HIGH);
                endTime1=millis();
                elapsedTime1 = (endTime1-startTime)/1000.0;
                mouse1Done=1;
              }
              if ((mouse2Done==0) && (mouse2Fell==HIGH))
              {
                digitalWrite(redLED2, LOW);
                digitalWrite(grnLED2, HIGH);
                digitalWrite(bluLED2, HIGH);
                endTime2=millis();
                elapsedTime2 = (endTime2-startTime)/1000.0;
                mouse2Done=1;
               }
              if ((mouse3Done==0) && (mouse3Fell==HIGH)) 
              {
               digitalWrite(redLED3, LOW);
               digitalWrite(grnLED3, HIGH);
               digitalWrite(bluLED3, HIGH);
               endTime3=millis();
               elapsedTime3 = (endTime3-startTime)/1000.0;
               mouse3Done=1;
              }  
              if ((mouse4Done==0) && (mouse4Fell==HIGH))
              {
                digitalWrite(redLED4, LOW);
                digitalWrite(grnLED4, HIGH);
                digitalWrite(bluLED4, HIGH);
                endTime4=millis();
                elapsedTime4 = (endTime4-startTime)/1000.0;
                mouse4Done=1;
              }
             } 
             break;                                   //breaks out of switch case sequence
       } 
       //********************************************CASE 2: 10rpm for 3 min ()**************************************************
        case 2:
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Timing...");
          digitalWrite(redLED0, LOW);
          digitalWrite(redLED1, LOW);
          digitalWrite(redLED2, LOW);
          digitalWrite(redLED3, LOW);
          digitalWrite(redLED4, LOW);
          digitalWrite(grnLED0, LOW);
          digitalWrite(grnLED1, LOW);
          digitalWrite(grnLED2, LOW);
          digitalWrite(grnLED3, LOW);
          digitalWrite(grnLED4, LOW);
          digitalWrite(bluLED0, LOW);
          digitalWrite(bluLED1, LOW);
          digitalWrite(bluLED2, LOW);
          digitalWrite(bluLED3, LOW);
          digitalWrite(bluLED4, LOW);
          delay(100);
          startTime = millis();
          timingStarted=1;
          digitalWrite(dirPin, LOW);
          while ((millis()-startTime < 180000) && (killButtonState==LOW) && ((mouse0Fell==LOW) || (mouse1Fell==LOW) || (mouse2Fell==LOW) || (mouse3Fell==LOW) || (mouse4Fell==LOW)))
          {
            digitalWrite(pulPin, HIGH);
            delay (2.030/radiusFactor);
            digitalWrite (pulPin, LOW);
            delay (2.030/radiusFactor);
            if ((mouse0Done==0) && (mouse0Fell==HIGH))
            {
              digitalWrite(redLED0, LOW);
              digitalWrite(grnLED0, HIGH);
              digitalWrite(bluLED0, HIGH);
              endTime0=millis();
              elapsedTime0 = (endTime0-startTime)/1000.0;
              mouse0Done=1;
            }
            if ((mouse1Done==0) && (mouse1Fell==HIGH))
            {
              digitalWrite(redLED1, LOW);
              digitalWrite(grnLED1, HIGH);
              digitalWrite(bluLED1, HIGH);
              endTime1=millis();
              elapsedTime1 = (endTime1-startTime)/1000.0;
              mouse1Done=1;
            }
            if ((mouse2Done==0) && (mouse2Fell==HIGH))
            {
              digitalWrite(redLED2, LOW);
              digitalWrite(grnLED2, HIGH);
              digitalWrite(bluLED2, HIGH);
              endTime2=millis();
              elapsedTime2 = (endTime2-startTime)/1000.0;
              mouse2Done=1;
            }
            if ((mouse3Done==0) && (mouse3Fell==HIGH)) 
            {
              digitalWrite(redLED3, LOW);
              digitalWrite(grnLED3, HIGH);
              digitalWrite(bluLED3, HIGH);
              endTime3=millis();
              elapsedTime3 = (endTime3-startTime)/1000.0;
              mouse3Done=1;
            }  
            if ((mouse4Done==0) && (mouse4Fell==HIGH))
            {
              digitalWrite(redLED4, LOW);
              digitalWrite(grnLED4, HIGH);
              digitalWrite(bluLED4, HIGH);
              endTime4=millis();
              elapsedTime4 = (endTime4-startTime)/1000.0;
              mouse4Done=1;
            }
           } 
           break;   
       }        
        //********************************************CASE 3: 4->60rpm in 300 sec **************************************************
        case 3:
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Timing...");
          digitalWrite(redLED0, LOW);
          digitalWrite(redLED1, LOW);
          digitalWrite(redLED2, LOW);
          digitalWrite(redLED3, LOW);
          digitalWrite(redLED4, LOW);
          digitalWrite(grnLED0, LOW);
          digitalWrite(grnLED1, LOW);
          digitalWrite(grnLED2, LOW);
          digitalWrite(grnLED3, LOW);
          digitalWrite(grnLED4, LOW);
          digitalWrite(bluLED0, LOW);
          digitalWrite(bluLED1, LOW);
          digitalWrite(bluLED2, LOW);
          digitalWrite(bluLED3, LOW);
          digitalWrite(bluLED4, LOW);
          delay(100);
          startTime = millis();
          timingStarted=1;
          while ((millis()-startTime < 300000) && (killButtonState==LOW) && ((mouse0Fell==LOW) || (mouse1Fell==LOW) || (mouse2Fell==LOW) || (mouse3Fell==LOW) || (mouse4Fell==LOW)))
          {
            currentElapsedTime = (millis()-startTime);
            delayTMillis = 1000.0*(60.0/(1600.0*((7.390*radiusFactor)+((103.466*radiusFactor)*currentElapsedTime/300000.0))));      //ramping equation -> delayT = 60/(160(RPM+((RPMf-RPMi)/Total Ramping Time)*elapsedTime))
            delayTMicros = delayTMillis*1000.0;
            if (delayTMillis > 5)                              //if the delay time is less than 5, we used micros, avoided further troubleshooting
            { 
               digitalWrite(pulPin, HIGH);
               delay (delayTMillis);
               digitalWrite (pulPin, LOW);
               delay (delayTMillis);
            }
            else
            {
               digitalWrite(pulPin, HIGH);
               delayMicroseconds (delayTMicros);
               digitalWrite (pulPin, LOW);
               delayMicroseconds (delayTMicros);
            }
            if ((mouse0Done==0) && (mouse0Fell==HIGH))
            {
              digitalWrite(redLED0, LOW);
              digitalWrite(grnLED0, HIGH);
              digitalWrite(bluLED0, HIGH);
              endTime0=millis();
              elapsedTime0 = (endTime0-startTime)/1000.0;
              mouse0Done=1;
            }
            if ((mouse1Done==0) && (mouse1Fell==HIGH))
            {
              digitalWrite(redLED1, LOW);
              digitalWrite(grnLED1, HIGH);
              digitalWrite(bluLED1, HIGH);
              endTime1=millis();
              elapsedTime1 = (endTime1-startTime)/1000.0;
              mouse1Done=1;
            }
            if ((mouse2Done==0) && (mouse2Fell==HIGH))
            {
              digitalWrite(redLED2, LOW);
              digitalWrite(grnLED2, HIGH);
              digitalWrite(bluLED2, HIGH);
              endTime2=millis();
              elapsedTime2 = (endTime2-startTime)/1000.0;
              mouse2Done=1;
             }  
             if ((mouse3Done==0) && (mouse3Fell==HIGH)) 
             {
               digitalWrite(redLED3, LOW);
               digitalWrite(grnLED3, HIGH);
               digitalWrite(bluLED3, HIGH);
               endTime3=millis();
               elapsedTime3 = (endTime3-startTime)/1000.0;
               mouse3Done=1;
              }     
              if ((mouse4Done==0) && (mouse4Fell==HIGH))
              {
                digitalWrite(redLED4, LOW);
                digitalWrite(grnLED4, HIGH);
                digitalWrite(bluLED4, HIGH);
                endTime4=millis();
                elapsedTime4 = (endTime4-startTime)/1000.0;
                mouse4Done=1;
              }
          }
          break;
          }
        //********************************************CASE 4: 4->60rpm in 150 sec**************************************************
        case 4:
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Timing...");
          digitalWrite(redLED0, LOW);
          digitalWrite(redLED1, LOW);
          digitalWrite(redLED2, LOW);
          digitalWrite(redLED3, LOW);
          digitalWrite(redLED4, LOW);
          digitalWrite(grnLED0, LOW);
          digitalWrite(grnLED1, LOW);
          digitalWrite(grnLED2, LOW);
          digitalWrite(grnLED3, LOW);
          digitalWrite(grnLED4, LOW);
          digitalWrite(bluLED0, LOW);
          digitalWrite(bluLED1, LOW);
          digitalWrite(bluLED2, LOW);
          digitalWrite(bluLED3, LOW);
          digitalWrite(bluLED4, LOW);
          delay(100);
          startTime = millis();
          timingStarted=1;
          while ((millis()-startTime < 150000) && (killButtonState==LOW) && ((mouse0Fell==LOW) || (mouse1Fell==LOW) || (mouse2Fell==LOW) || (mouse3Fell==LOW) || (mouse4Fell==LOW)))
          {
            currentElapsedTime = (millis()-startTime);
            delayTMillis = 1000.0*(60.0/(1600.0*((7.390*radiusFactor)+((103.466*radiusFactor)*currentElapsedTime/150000.0))));
            delayTMicros = delayTMillis*1000.0;
            if (delayTMillis > 5)
            {
                digitalWrite(pulPin, HIGH);
                delay (delayTMillis);
                digitalWrite (pulPin, LOW);
                delay (delayTMillis);
            }
            else
            {
                digitalWrite(pulPin, HIGH);
                delayMicroseconds (delayTMicros);
                digitalWrite (pulPin, LOW);
                delayMicroseconds (delayTMicros);
            }
            if ((mouse0Done==0) && (mouse0Fell==HIGH))
            {
                digitalWrite(redLED0, LOW);
                digitalWrite(grnLED0, HIGH);
                digitalWrite(bluLED0, HIGH);
                endTime0=millis();
                elapsedTime0 = (endTime0-startTime)/1000.0;
                mouse0Done=1;
            }
            if ((mouse1Done==0) && (mouse1Fell==HIGH))
            {
                digitalWrite(redLED1, LOW);
                digitalWrite(grnLED1, HIGH);
                digitalWrite(bluLED1, HIGH);
                endTime1=millis();
                elapsedTime1 = (endTime1-startTime)/1000.0;
                mouse1Done=1;
            }
            if ((mouse2Done==0) && (mouse2Fell==HIGH))
            {
                digitalWrite(redLED2, LOW);
                digitalWrite(grnLED2, HIGH);
                digitalWrite(bluLED2, HIGH);
                endTime2=millis();
                elapsedTime2 = (endTime2-startTime)/1000.0;
                mouse2Done=1;
            }        
            if ((mouse3Done==0) && (mouse3Fell==HIGH)) 
            {
                digitalWrite(redLED3, LOW);
                digitalWrite(grnLED3, HIGH);
                digitalWrite(bluLED3, HIGH);
                endTime3=millis();
                elapsedTime3 = (endTime3-startTime)/1000.0;
                mouse3Done=1;
             }  
             if ((mouse4Done==0) && (mouse4Fell==HIGH))
             {
                digitalWrite(redLED4, LOW);
                digitalWrite(grnLED4, HIGH);
                digitalWrite(bluLED4, HIGH);
                endTime4=millis();
                elapsedTime4 = (endTime4-startTime)/1000.0;
                mouse4Done=1;
             }   
           }
          break;
        }
        //********************************************CASE 5: 4->60rpm in 300 sec **************************************************
        case 5:
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Timing...");
          digitalWrite(redLED0, LOW);
          digitalWrite(redLED1, LOW);
          digitalWrite(redLED2, LOW);
          digitalWrite(redLED3, LOW);
          digitalWrite(redLED4, LOW);
          digitalWrite(grnLED0, LOW);
          digitalWrite(grnLED1, LOW);
          digitalWrite(grnLED2, LOW);
          digitalWrite(grnLED3, LOW);
          digitalWrite(grnLED4, LOW);
          digitalWrite(bluLED0, LOW);
          digitalWrite(bluLED1, LOW);
          digitalWrite(bluLED2, LOW);
          digitalWrite(bluLED3, LOW);
          digitalWrite(bluLED4, LOW);
          delay(100);
          startTime = millis();
          timingStarted=1;
          digitalWrite(dirPin,HIGH);
          while ((millis()-startTime < 300000) && (killButtonState==LOW) && ((mouse0Fell==LOW) || (mouse1Fell==LOW) || (mouse2Fell==LOW) || (mouse3Fell==LOW)|| (mouse4Fell==LOW)))
          {
            currentElapsedTime = (millis()-startTime);
            delayTMillis = 1000.0*(60.0/(1600.0*((7.390*radiusFactor)+((103.466*radiusFactor)*currentElapsedTime/300000.0))));
            delayTMicros = delayTMillis*1000.0;
            if (delayTMillis > 5)
            {
                digitalWrite(pulPin, HIGH);
                delay (delayTMillis);
                digitalWrite (pulPin, LOW);
                delay (delayTMillis);
            }
            else
            {
                digitalWrite(pulPin, HIGH);
                delayMicroseconds (delayTMicros);
                digitalWrite (pulPin, LOW);
                delayMicroseconds (delayTMicros);
            }
            if ((mouse0Done==0) && (mouse0Fell==HIGH))
            {
                digitalWrite(redLED0, LOW);
                digitalWrite(grnLED0, HIGH);
                digitalWrite(bluLED0, HIGH);
                endTime0=millis();
                elapsedTime0 = (endTime0-startTime)/1000.0;
                mouse0Done=1;
            }
            if ((mouse1Done==0) && (mouse1Fell==HIGH))
            {
                digitalWrite(redLED1, LOW);
                digitalWrite(grnLED1, HIGH);
                digitalWrite(bluLED1, HIGH);
                endTime1=millis();
                elapsedTime1 = (endTime1-startTime)/1000.0;
                mouse1Done=1;
            }
            if ((mouse2Done==0) && (mouse2Fell==HIGH))
            {
                digitalWrite(redLED2, LOW);
                digitalWrite(grnLED2, HIGH);
                digitalWrite(bluLED2, HIGH);
                endTime2=millis();
                elapsedTime2 = (endTime2-startTime)/1000.0;
                mouse2Done=1;
            } 
            if ((mouse3Done==0) && (mouse3Fell==HIGH)) 
            {
                digitalWrite(redLED3, LOW);
                digitalWrite(grnLED3, HIGH);
                digitalWrite(bluLED3, HIGH);
                endTime3=millis();
                elapsedTime3 = (endTime3-startTime)/1000.0;
                mouse3Done=1;
            }   
            if ((mouse4Done==0) && (mouse4Fell==HIGH))
            {
                digitalWrite(redLED4, LOW);
                digitalWrite(grnLED4, HIGH);
                digitalWrite(bluLED4, HIGH);
                endTime4=millis();
                elapsedTime4 = (endTime4-startTime)/1000.0;
                mouse4Done=1;
            }  
           }
          break;
        }
        //***********CASE 6: 1->30 rpm (1 min); 30 rpm (15 s); 30->1 rpm (1 min); -1->-15 rpm (1 min); -15->-1 rpm (1 min) *********************
        case 6: 
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Timing...");
          startTime = millis();
          timingStarted=1;
          digitalWrite(redLED0, LOW);
          digitalWrite(redLED1, LOW);
          digitalWrite(redLED2, LOW);
          digitalWrite(redLED3, LOW);
          digitalWrite(redLED4, LOW);
          digitalWrite(grnLED0, LOW);
          digitalWrite(grnLED1, LOW);
          digitalWrite(grnLED2, LOW);
          digitalWrite(grnLED3, LOW);
          digitalWrite(grnLED4, LOW);
          digitalWrite(bluLED0, LOW);
          digitalWrite(bluLED1, LOW);
          digitalWrite(bluLED2, LOW);
          digitalWrite(bluLED3, LOW);
          digitalWrite(bluLED4, LOW);
          delay(100);
          //---------------------------------------- 1->30rpm in 1 minute --------------------------------------------------- //To do ramping rates and speeds for different time lengths, we utilized (millis()-starTime < 60000 milliseconds) such that each while loop will be active at different times
           while ((millis()-startTime < 60000) && ((mouse0Fell==LOW) || (mouse1Fell==LOW) || (mouse2Fell==LOW) || (mouse3Fell==LOW) || (mouse4Fell==LOW)) && (killButtonState==LOW))
           {
            currentElapsedTime = (millis()-startTime);
            delayTMillis = 1000.0*(60.0/(1600.0*((1.848*radiusFactor)+((53.944*radiusFactor)*currentElapsedTime/60000.0))));
            delayTMicros = delayTMillis*1000.0;
            if (delayTMillis > 5)
            {
               digitalWrite(pulPin, HIGH);
               delay (delayTMillis);
               digitalWrite (pulPin, LOW);
               delay (delayTMillis);
            }
            else
            {
               digitalWrite(pulPin, HIGH);
               delayMicroseconds (delayTMicros);
               digitalWrite (pulPin, LOW);
               delayMicroseconds (delayTMicros);
            }
            if ((mouse0Done==0) && (mouse0Fell==HIGH))
            {
               digitalWrite(redLED0, LOW);
               digitalWrite(grnLED0, HIGH);
               digitalWrite(bluLED0, HIGH);
               endTime0=millis();
               elapsedTime0 = (endTime0-startTime)/1000.0;
               mouse0Done=1;
            }
            if ((mouse1Done==0) && (mouse1Fell==HIGH))
            {
               digitalWrite(redLED1, LOW);
               digitalWrite(grnLED1, HIGH);
               digitalWrite(bluLED1, HIGH);
               endTime1=millis();
               elapsedTime1 = (endTime1-startTime)/1000.0;
               mouse1Done=1;
            }
            if ((mouse2Done==0) && (mouse2Fell==HIGH))
            {
               digitalWrite(redLED2, LOW);
               digitalWrite(grnLED2, HIGH);
               digitalWrite(bluLED2, HIGH);
               endTime2=millis();
               elapsedTime2 = (endTime2-startTime)/1000.0;
               mouse2Done=1;
            }
            if ((mouse3Done==0) && (mouse3Fell==HIGH)) 
            {
               digitalWrite(redLED3, LOW);
               digitalWrite(grnLED3, HIGH);
               digitalWrite(bluLED3, HIGH);
               endTime3=millis();
               elapsedTime3 = (endTime3-startTime)/1000.0;
               mouse3Done=1;
             }
             if ((mouse4Done==0) && (mouse4Fell==HIGH))
             {
               digitalWrite(redLED4, LOW);
               digitalWrite(grnLED4, HIGH);
               digitalWrite(bluLED4, HIGH);
               endTime4=millis();
               elapsedTime4 = (endTime4-startTime)/1000.0;
               mouse4Done=1;
             }
         } 
             //-------------------------------------------30rpm for 15 seconds---------------------------------------------------------
             while ((millis()-startTime < 75000) && ((mouse0Fell==LOW) || (mouse1Fell==LOW) || (mouse2Fell==LOW) || (mouse3Fell==LOW) || (mouse4Fell==LOW)) && (killButtonState==LOW))
             {
              digitalWrite(pulPin, HIGH);
              delayMicroseconds (676.553/radiusFactor);
              digitalWrite (pulPin, LOW);
              delayMicroseconds (676.553/radiusFactor);
              if ((mouse0Done==0) && (mouse0Fell==HIGH))
              {
                  digitalWrite(redLED0, LOW);
                  digitalWrite(grnLED0, HIGH);
                  digitalWrite(bluLED0, HIGH);
                  endTime0=millis();
                  elapsedTime0 = (endTime0-startTime)/1000.0;
                  mouse0Done=1;
              }
              if ((mouse1Done==0) && (mouse1Fell==HIGH))
              {
                  digitalWrite(redLED1, LOW);
                  digitalWrite(grnLED1, HIGH);
                  digitalWrite(bluLED1, HIGH);
                  endTime1=millis();
                  elapsedTime1 = (endTime1-startTime)/1000.0;
                  mouse1Done=1;
              }
              if ((mouse2Done==0) && (mouse2Fell==HIGH))
              {
                  digitalWrite(redLED2, LOW);
                  digitalWrite(grnLED2, HIGH);
                  digitalWrite(bluLED2, HIGH);
                  endTime2=millis();
                  elapsedTime2 = (endTime2-startTime)/1000.0;
                  mouse2Done=1;
              }
              if ((mouse3Done==0) && (mouse3Fell==HIGH)) 
              {
                  digitalWrite(redLED3, LOW);
                  digitalWrite(grnLED3, HIGH);
                  digitalWrite(bluLED3, HIGH);
                  endTime3=millis();
                  elapsedTime3 = (endTime3-startTime)/1000.0;
                  mouse3Done=1;
              }
              if ((mouse4Done==0) && (mouse4Fell==HIGH))
              {
                  digitalWrite(redLED4, LOW);
                  digitalWrite(grnLED4, HIGH);
                  digitalWrite(bluLED4, HIGH);
                  endTime4=millis();
                  elapsedTime4 = (endTime4-startTime)/1000.0;
                  mouse4Done=1;
              }
             }
             //-------------------------------------------30->1rpm in 60 seconds-------------------------------------------------------------
             while ((millis()-startTime < 135000) && ((mouse0Fell==LOW) || (mouse1Fell==LOW) || (mouse2Fell==LOW) || (mouse3Fell==LOW) || (mouse4Fell==LOW)) && (killButtonState==LOW))
             {
              currentElapsedTime = (millis()-startTime);
              delayTMillis = 1000.0*(60.0/(1600.0*((55.428*radiusFactor)-((53.944*radiusFactor)*(currentElapsedTime-75000.0)/60000.0))));
              delayTMicros = delayTMillis*1000.0;
              if (delayTMillis > 5)
              {
                  digitalWrite(pulPin, HIGH);
                  delay (delayTMillis);
                  digitalWrite (pulPin, LOW);
                 delay (delayTMillis);
               }
              else
              {
                  digitalWrite(pulPin, HIGH);
                  delayMicroseconds (delayTMicros);
                  digitalWrite (pulPin, LOW);
                  delayMicroseconds (delayTMicros);
              }
              if ((mouse0Done==0) && (mouse0Fell==HIGH))
              {
                  digitalWrite(redLED0, LOW);
                  digitalWrite(grnLED0, HIGH);
                  digitalWrite(bluLED0, HIGH);
                  endTime0=millis();
                  elapsedTime0 = (endTime0-startTime)/1000.0;
                  mouse0Done=1;
              }
              if ((mouse1Done==0) && (mouse1Fell==HIGH))
              {
                    digitalWrite(redLED1, LOW);
                    digitalWrite(grnLED1, HIGH);
                    digitalWrite(bluLED1, HIGH);
                    endTime1=millis();
                    elapsedTime1 = (endTime1-startTime)/1000.0;
                    mouse1Done=1;
              }
              if ((mouse2Done==0) && (mouse2Fell==HIGH))
              {
                    digitalWrite(redLED2, LOW);
                    digitalWrite(grnLED2, HIGH);
                    digitalWrite(bluLED2, HIGH);
                    endTime2=millis();
                    elapsedTime2 = (endTime2-startTime)/1000.0;
                    mouse2Done=1;
              }
              if ((mouse3Done==0) && (mouse3Fell==HIGH)) 
              {
                    digitalWrite(redLED3, LOW);
                    digitalWrite(grnLED3, HIGH);
                    digitalWrite(bluLED3, HIGH);
                    endTime3=millis();
                    elapsedTime3 = (endTime3-startTime)/1000.0;
                    mouse3Done=1;
              }
              if ((mouse4Done==0) && (mouse4Fell==HIGH))
              {
                    digitalWrite(redLED4, LOW);
                    digitalWrite(grnLED4, HIGH);
                    digitalWrite(bluLED4, HIGH);
                    endTime4=millis();
                    elapsedTime4 = (endTime4-startTime)/1000.0;
                    mouse4Done=1;
              }
             }  
             //-----------------------------------------------(-1->-15 rpm in 60 seconds)--------------------------------------------------
             while ((millis()-startTime < 195000) && ((mouse0Fell==LOW) || (mouse1Fell==LOW) || (mouse2Fell==LOW) || (mouse3Fell==LOW) || (mouse4Fell==LOW)) && (killButtonState==LOW))
             {
              currentElapsedTime = (millis()-startTime); 
              delayTMillis = 1000.0*(60.0/(1600.0*((1.484*radiusFactor)+((26.230*radiusFactor)*(currentElapsedTime-135000.0)/60000.0))));
              delayTMicros = delayTMillis*1000.0;
              digitalWrite(dirPin,HIGH);
              if (delayTMillis > 5)
              {
                  digitalWrite(pulPin, HIGH);
                  delay (delayTMillis);
                  digitalWrite (pulPin, LOW);
                  delay (delayTMillis);
              }
              else
              {
                  digitalWrite(pulPin, HIGH);
                  delayMicroseconds (delayTMicros);
                  digitalWrite (pulPin, LOW);
                  delayMicroseconds (delayTMicros);
              }
              if ((mouse0Done==0) && (mouse0Fell==HIGH))
              {
                  digitalWrite(redLED0, LOW);
                  digitalWrite(grnLED0, HIGH);
                  digitalWrite(bluLED0, HIGH);
                  endTime0=millis();
                  elapsedTime0 = (endTime0-startTime)/1000.0;
                  mouse0Done=1;
              }
              if ((mouse1Done==0) && (mouse1Fell==HIGH))
              {
                  digitalWrite(redLED1, LOW);
                  digitalWrite(grnLED1, HIGH);
                  digitalWrite(bluLED1, HIGH);
                  endTime1=millis();
                  elapsedTime1 = (endTime1-startTime)/1000.0;
                  mouse1Done=1;
              }
              if ((mouse2Done==0) && (mouse2Fell==HIGH))
              {
                  digitalWrite(redLED2, LOW);
                  digitalWrite(grnLED2, HIGH);
                  digitalWrite(bluLED2, HIGH);
                  endTime2=millis();
                  elapsedTime2 = (endTime2-startTime)/1000.0;
                  mouse2Done=1;
              }         
              if ((mouse3Done==0) && (mouse3Fell==HIGH)) 
              {
                  digitalWrite(redLED3, LOW);
                  digitalWrite(grnLED3, HIGH);
                  digitalWrite(bluLED3, HIGH);
                  endTime3=millis();
                  elapsedTime3 = (endTime3-startTime)/1000.0;
                  mouse3Done=1;
              }
              if ((mouse4Done==0) && (mouse4Fell==HIGH))
              {
                  digitalWrite(redLED4, LOW);
                  digitalWrite(grnLED4, HIGH);
                  digitalWrite(bluLED4, HIGH);
                  endTime4=millis();
                  elapsedTime4 = (endTime4-startTime)/1000.0;
                  mouse4Done=1;
              }      
             }
             //-----------------------------------------------(-15->-1rpm in 60 seconds)--------------------------------------------------
             while ((millis()-startTime < 255000) && ((mouse0Fell==LOW) || (mouse1Fell==LOW) || (mouse2Fell==LOW) || (mouse3Fell==LOW) || (mouse4Fell==LOW)) && (killButtonState==LOW))
             {
                  Serial.println("In last loop");
                  currentElapsedTime = (millis()-startTime);
                  delayTMillis = 1000.0*(60.0/(1600.0*((27.714*radiusFactor)-((26.230*radiusFactor)*(currentElapsedTime-195000.0)/60000.0))));
                  delayTMicros = delayTMillis*1000.0;
                  if (delayTMillis > 5)
                  {
                      digitalWrite(pulPin, HIGH);
                      delay (delayTMillis);
                      digitalWrite (pulPin, LOW);
                      delay (delayTMillis);
                  }
                  else
                  {
                      digitalWrite(pulPin, HIGH);
                      delayMicroseconds (delayTMicros);
                      digitalWrite (pulPin, LOW);
                      delayMicroseconds (delayTMicros);
                  }
                  if ((mouse0Done==0) && (mouse0Fell==HIGH))
                  {
                      digitalWrite(redLED0, LOW);
                      digitalWrite(grnLED0, HIGH);
                      digitalWrite(bluLED0, HIGH);
                      endTime0=millis();
                      elapsedTime0 = (endTime0-startTime)/1000.0;
                      mouse0Done=1;
                  }
                  if ((mouse1Done==0) && (mouse1Fell==HIGH))
                  {
                      digitalWrite(redLED1, LOW);
                      digitalWrite(grnLED1, HIGH);
                      digitalWrite(bluLED1, HIGH);
                      endTime1=millis();
                      elapsedTime1 = (endTime1-startTime)/1000.0;
                      mouse1Done=1;
                  }
                  if ((mouse2Done==0) && (mouse2Fell==HIGH))
                  {
                      digitalWrite(redLED2, LOW);
                      digitalWrite(grnLED2, HIGH);
                      digitalWrite(bluLED2, HIGH);
                      endTime2=millis();
                      elapsedTime2 = (endTime2-startTime)/1000.0;
                      mouse2Done=1;
                  }
                  if ((mouse3Done==0) && (mouse3Fell==HIGH)) 
                  {
                      digitalWrite(redLED3, LOW);
                      digitalWrite(grnLED3, HIGH);
                      digitalWrite(bluLED3, HIGH);
                      endTime3=millis();
                      elapsedTime3 = (endTime3-startTime)/1000.0;
                      mouse3Done=1;
                  }
                  if ((mouse4Done==0) && (mouse4Fell==HIGH))
                  {
                      digitalWrite(redLED4, LOW);
                      digitalWrite(grnLED4, HIGH);
                      digitalWrite(bluLED4, HIGH);
                      endTime4=millis();
                      elapsedTime4 = (endTime4-startTime)/1000.0;
                      mouse4Done=1;
                  }
            }
          break;
        }
      }
        digitalWrite(redLED0, LOW);                 //RGB LEDs to red
        digitalWrite(redLED1, LOW);
        digitalWrite(redLED2, LOW);
        digitalWrite(redLED3, LOW);
        digitalWrite(redLED4, LOW);
        digitalWrite(grnLED0, HIGH);
        digitalWrite(grnLED1, HIGH);
        digitalWrite(grnLED2, HIGH);
        digitalWrite(grnLED3, HIGH);
        digitalWrite(grnLED4, HIGH);
        digitalWrite(bluLED0, HIGH);
        digitalWrite(bluLED1, HIGH);
        digitalWrite(bluLED2, HIGH);
        digitalWrite(bluLED3, HIGH);
        digitalWrite(bluLED4, HIGH);
        mouse0Done=0;                               //Resetting State Variables
        mouse1Done=0;
        mouse2Done=0;
        mouse3Done=0;
        mouse4Done=0;
        timingStarted=0;
        lcd.clear();
        lcd.setCursor(0,0);                         //NOTE - ET1 correlates to ET4 in code, ET5 on SM correlates to ET0 in code
        lcd.print("ET1: ");                         //Printing elapsed time on LCD
        lcd.setCursor(5,0);
        lcd.print(elapsedTime4);
        lcd.setCursor(0,1);
        lcd.print("ET2: ");
        lcd.setCursor(5,1);
        lcd.print(elapsedTime3);
        lcd.setCursor(0,2);
        lcd.print("ET3: ");
        lcd.setCursor(5,2);
        lcd.print(elapsedTime2);
        lcd.setCursor(0,3);
        lcd.print("ET4: ");
        lcd.setCursor(5,3);
        lcd.print(elapsedTime1);
        lcd.setCursor(11,0);
        lcd.print("ET5: ");
        lcd.setCursor(15,0);
        lcd.print(elapsedTime0);
        startButtonState=0;                         //Resetting state variables
        killButtonState = LOW;
        mouse0Fell=LOW;
        mouse1Fell=LOW;
        mouse2Fell=LOW;
        mouse3Fell=LOW;
        mouse4Fell=LOW;
        digitalWrite (dirPin, LOW);
 }
} //end of void loop()

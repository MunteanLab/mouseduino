/*This code is for the Arduino UNO - Secondary Microcontroller
 Written by J. Widjaja and J. A. Hauger at Augusta University in January 2022.

 Program Description: Receives sensor inputs, processes value, and sends out signals to Arduino MEGA alerting is mouse has fallen. Acts as a comparator chip.
*/

//initializing variables
float currentVolts0 = 0;          //Voltage reading after processed through equation Volts = (5.0*sensor reading)/1023.0
float currentVolts1 = 0;
float currentVolts2 = 0;
float currentVolts3 = 0;
float currentVolts4 = 0;
float previousVolts0 = 0;         //Variables used for digital lowpass filter
float previousVolts1 = 0;
float previousVolts2 = 0;
float previousVolts3 = 0;
float previousVolts4 = 0;
float newVolts0 = 0;
float newVolts1 = 0;
float newVolts2 = 0;
float newVolts3 = 0;
float newVolts4 = 0;
float a=0.92;                 

void setup() {
pinMode(2,OUTPUT);              //Set pins 2-6 as output pins connected to Arduino MEGA
pinMode(3,OUTPUT);
pinMode(4,OUTPUT);
pinMode(5,OUTPUT);
pinMode(6,OUTPUT);
}

void loop() {
  currentVolts0 = 5.0*analogRead(A0)/1023.0;                    //Received and converts analog signals to voltage for sensors 0-4
  currentVolts1 = 5.0*analogRead(A1)/1023.0;
  currentVolts2 = 5.0*analogRead(A2)/1023.0;
  currentVolts3 = 5.0*analogRead(A3)/1023.0;
  currentVolts4 = 5.0*analogRead(A4)/1023.0;
  newVolts0 = (a*previousVolts0)+((1.0-a)*(currentVolts0));     //Digital Lowpass Filter - reduces noise from sensor to prevent false readings
  newVolts1 = (a*previousVolts1)+((1.0-a)*(currentVolts1));
  newVolts2 = (a*previousVolts2)+((1.0-a)*(currentVolts2));
  newVolts3 = (a*previousVolts3)+((1.0-a)*(currentVolts3));
  newVolts4 = (a*previousVolts4)+((1.0-a)*(currentVolts4));
  previousVolts0 = newVolts0;
  previousVolts1 = newVolts1;
  previousVolts2 = newVolts2;
  previousVolts3 = newVolts3;
  previousVolts4 = newVolts4;

  //Logic
  if(newVolts0>2.0)                                             //If processed voltage reading is greater than 2, mouse has fallen and signal is sent to Arduino MEGA
  {                                                             //Can change number (2.0) depending on sensor sensitivity
    digitalWrite(2,LOW);
    delay(1);
    digitalWrite(2,HIGH);
    delay(1);
    digitalWrite(2,LOW);
  }
  if(newVolts1>2.0)
  {
    digitalWrite(3,LOW);
    delay(1);
    digitalWrite(3,HIGH);
    delay(1);
    digitalWrite(3,LOW);
  }
  if(newVolts2>2.0)
  {
    digitalWrite(4,LOW);
    delay(1);
    digitalWrite(4,HIGH);
    delay(1);
    digitalWrite(4,LOW);
  }
  if(newVolts3>2.0)
  {
    digitalWrite(5,LOW);
    delay(1);
    digitalWrite(5,HIGH);
    delay(1);
    digitalWrite(5,LOW);
  }

   if(newVolts4>2.0)
  {
    digitalWrite(6,LOW);
    delay(1);
    digitalWrite(6,HIGH);
    delay(1);
    digitalWrite(6,LOW);
  } 
}

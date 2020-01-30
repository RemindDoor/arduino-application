
#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position

const int buttonPin = 2;     // the number of the pushbutton pin

// variables will change:
int buttonState = 0; 

boolean turnedLeft = false;
boolean buttonPressed = false;

void setup() {
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == LOW && !buttonPressed) {
    buttonPressed = true;
    if (turnedLeft) {
      
       // goes from 180 degrees to 0 degrees
        myservo.write(0);              // tell servo to go to position in variable 'pos'
        delay(150);                    // waits 15ms for the servo to reach the position
      
      turnedLeft = false;
    } else {
       // goes from 0 degrees to 180 degrees
        // in steps of 1 degree
        myservo.write(90);
        delay(150);        // tell servo to go to position in variable 'pos'
        
         // waits 15ms for the servo to reach the position
      
  
      turnedLeft = true;
    }
    delay(100);
  } else if (buttonState != LOW) {
    if (buttonPressed) {
      buttonPressed = false;  
    }
    // do nothing
  }
}

#include <Servo.h>
#include <Stepper.h>

const int ledPin = 13;
const byte buffSize = 40;
unsigned int inputBuffer[buffSize];
const char startMarker = '<';
const char endMarker = '>';
byte bytesRecvd = 0;
boolean readInProgress = false;
boolean newDataFromPC = false;
byte coordinates[3];
int radius = 0, angle = 0, shape = 0;
int oldRadius = 0, oldAngle = 0, force = 0;
int offset = 280;
Stepper rotate(200,8,9,10,11);
Servo magnet;

void setup() //Setup Stuff
{
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  rotate.setSpeed(2);
  magnet.attach(12);
  goHome();
  NOMAGNETEYES();
}

void loop() 
{
  getDataFromPC();
  if(newDataFromPC)
  {
    sendSuspendCmd();
    digitalWrite(ledPin, HIGH);

    //Moving to coordinates
    rotate.step(-95);
    rotate.step(angle);
    horzontialForwards(radius);

    //Grab the shape
    magnet.write(0);
    MAGNETEYES();
    delay(1500);

    
    //Moving home
    goHome();

    //Sorting shapes
    dropOff(shape);

    //Wait for next command
    digitalWrite(ledPin, LOW);
    sendEnableCmd();
    newDataFromPC = false;
  }
}

//Return the arm to the Home position
void goHome()
{
  magnet.write(90);
  horzontialBackwards();
  while(digitalRead(2) == LOW)
  {
    rotate.step(1);
  }
}

//Magnet control functions
void MAGNETEYES()
{
  digitalWrite(5, HIGH);
  digitalWrite(4, LOW);
}
void NOMAGNETEYES()
{
  digitalWrite(5, LOW);
  digitalWrite(4, LOW);
}

//Radial position functions
void horzontialForwards(int duration)
{
  //Scale the duration into the correct movement time
  duration = duration * 100;
  digitalWrite(6, HIGH);
  digitalWrite(7, LOW);
  delay(duration);
  digitalWrite(6, LOW);
}
void horzontialBackwards(int duration)
{
  //Scale the duration into the correct movement time
  duration = duration * 100;
  digitalWrite(6, LOW);
  digitalWrite(7, HIGH);
  delay(duration);
  digitalWrite(7, LOW);
}
void horzontialBackwards()
{
  while(digitalRead(3) == LOW)
  {
    digitalWrite(6, LOW);
    digitalWrite(7, HIGH);
  }
  digitalWrite(7, LOW);
}

//Shape sorting functions
void dropOff(int polygon)
{
  if(polygon == 1)
  {
    Triangle();
  }
  else if(polygon == 2)
  {
    Rectangle();
  }
  else
  {
    NOMAGNETEYES();
  }
}
void Triangle()
{
  NOMAGNETEYES();
  delay(500);
}
void Rectangle()
{
  rotate.step(-1 * offset);
  NOMAGNETEYES();
  delay(500);
  goHome();
}

void sendSuspendCmd()
{
  // send the suspend-true command
  Serial.println("<S1>");
}

void sendEnableCmd()
{
  // send the suspend-false command
  Serial.println("<S0>");
}

void getDataFromPC() 
{
  //Testing input block
  /*
  if(Serial.available() > 0)
  {
    newDataFromPC = true;
    radius = Serial.parseInt();
    angle = Serial.parseInt();
    shape = Serial.parseInt();
    angle = (-1 * angle) - 0;
    Serial.println(radius);
    Serial.println(angle);
  }
  */
  
  // receive data from PC and save it into inputBuffer
  ///*
  if(Serial.available() > 0)
  {
    char x = Serial.read();
    
    // the order of these IF clauses is significant
    if (x == endMarker)
    {
      readInProgress = false;
      newDataFromPC = true;
      inputBuffer[bytesRecvd] = 0;
      coordinates[0] = inputBuffer[0];
      coordinates[1] = inputBuffer[1];
      coordinates[2] = inputBuffer[2];
      radius = (int)coordinates[0];
      angle = (int)coordinates[1];
      shape = (int)coordinates[2];
      angle = (-2 * angle) - 20;
      //BruteForce(radius, angle);
    }
    
    if(readInProgress)
    {
      inputBuffer[bytesRecvd] = x;
      bytesRecvd ++;
      if (bytesRecvd == buffSize)
      {
        bytesRecvd = buffSize - 1;
      }
    }
    
    if (x == startMarker)
    {
      bytesRecvd = 0;
      readInProgress = true;
    }
  }
  //*/
}

//Function to increment the angle if the coordante is similar to the last, in order to try and grab trouble spots
void BruteForce(int RCoord, int ACoord)
{
  if(force < 3)
  {
    if(RCoord < oldRadius + 10 && RCoord > oldRadius - 10)
    {
      if(ACoord < oldAngle + 10 && ACoord > oldAngle - 10)
      {
        angle = angle + 5;
        force++;
      }
    }
    
    else
    {
      force = 0;
      oldRadius = RCoord;
      oldAngle = ACoord;
    }
  }
}

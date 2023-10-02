#include<SoftwareSerial.h>
#include<Servo.h>

char OK[5] = "11111";

int total_people = 0;

Servo servoMoteur; 
  // on crée un objet servo appelé servoMoteur 
int led_1 = 4;
int led_2 = 7;
int led_people = 8;

int TRIGGER_PIN = 9;
int ECHO_PIN = 10;

#define MAX_DISTANCE 20

#define RxD 3
#define TxD 2

#define RxD_2 A2
#define TxD_2 A3

int TRIGGER_PIN_2 = 5;
int ECHO_PIN_2 = 6;
#define MAX_DISTANCE_2 20

int cnt_people = 0;

int sensor1 = 0;
int sensor2 = 0;


int motorPin = 11;   //Motor drive pin D3
int motorSpeed;     //Define motor speed


void sendFromBth1(String message) {
  SoftwareSerial BTSerie(RxD, TxD);
  BTSerie.begin(38400);
  BTSerie.println(message);
}

// String readFromBth1() {
//   SoftwareSerial BTSerie(RxD, TxD);
//   BTSerie.begin(38400);
//   String word="";
//   char c;

//   while(!BTSerie.available()) {}

//   delay(100);  

//   //BTSerie.println("miau2");
//   while(BTSerie.available()) {
//     c = BTSerie.read();
//     word += c;
//   }

//   return word;
// }

void sendFromBth2(String message) {
  SoftwareSerial BTSerie(RxD_2, TxD_2);
  BTSerie.begin(38400);
  BTSerie.print(message);
}

void setup() {
 // pinMode(motorPin, OUTPUT);
  servoMoteur.attach(11); 

  pinMode(led_1, OUTPUT);
  pinMode(led_2, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);  
	pinMode(ECHO_PIN, INPUT); 

  pinMode(TRIGGER_PIN_2, OUTPUT);  
	pinMode(ECHO_PIN_2, INPUT);  

	Serial.begin(9600); 

  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);

  pinMode(RxD_2, INPUT);
  pinMode(TxD_2, OUTPUT);
}

float getDistanceFromSensor(int TRIGGER_PIN, int ECHO_PIN) {
  float duration, distance;

  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration*.0343)/2;

  return distance;
}

void loop() {
  float distance1, distance2;

  if(cnt_people > 0){
    digitalWrite(led_people, HIGH);
  }
  else {
    digitalWrite(led_people, LOW);
  }

  distance1 = getDistanceFromSensor(TRIGGER_PIN, ECHO_PIN);
  distance2 = getDistanceFromSensor(TRIGGER_PIN_2, ECHO_PIN_2);

  if(distance1 < 10 && distance1 > 0)
  {
    if(sensor2 == 0 && sensor1 == 0)
    {
      sendFromBth2("o");
      digitalWrite(led_1, HIGH);
      sensor1 = 1;
    }
    else if(sensor2 == 1)
    {
      if(cnt_people >= 1) 
        cnt_people -= 1;
      sendFromBth1("Distance_2: " + String(distance2));
      //BTSerie.println(distance2);
      sendFromBth1("Counter: " + String(cnt_people));
      //BTSerie.println(cnt_people);
      sendFromBth2(String(total_people));
      sensor1 = 0;
      sensor2 = 0;
      delay(500);
    }
  }

  if(distance2 < 10 && distance2 > 0)
  {
    if(sensor1 == 0 && sensor2 == 0)
    {
      sendFromBth2("o");
      sensor2 = 1;
    }
    else if(sensor1 == 1)
    {
      digitalWrite(led_1, LOW);
      cnt_people += 1;
      total_people += 1;
      sendFromBth1("Distance_1: " + String(distance1));
      //BTSerie.println(distance2);
      sendFromBth1("Counter: " + String(cnt_people));
      //BTSerie.println(cnt_people);
      sendFromBth2(String(total_people));
      sensor1 = 0;
      sensor2 = 0;
      delay(500);
    }
  }
      
  if(cnt_people >= 2)
  { 
    //Serial.write(OK, 5);
    sendFromBth2("-");
    //analogWrite(motorPin, 128);
  }
  // else {
  //   Serial.write(notOK, 5);
  //   //analogWrite(motorPin, 0);
  // }

  //delay(1000);
  delay(1000);           
}
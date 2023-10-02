#include<SoftwareSerial.h>
#include<LiquidCrystal_I2C.h>
#include<string.h>
#include <Servo.h> 
#include <math.h>
#include <stdio.h>

LiquidCrystal_I2C lcd(0x20,16,2);

#define A_X 3
#define X_THRESHOLD 200  // Adjust this threshold value as needed
const int buzzerPin = 6; // Declaring the PWM pin for the buzzer

int val_x;
int prev_x;

int motorPin = 11;
int ok = 0;

int dangerFlame = 0;
int dangerEarthquake = 0;
int dangerGas = 0;

int total_people = 0;

int servoPin = 9;

unsigned long startMillis;  
unsigned long fireTime = 0;
unsigned long gasTime = 0;

#define RxD 12
#define TxD 13

#define TempSensor A0

SoftwareSerial BTSerie(RxD, TxD);

Servo servo;

void setup() {
  // Begin the Serial at 9600 Baud
  Serial.begin(9600);

  BTSerie.begin(38400);

  pinMode(A_X, INPUT);
  pinMode(buzzerPin, OUTPUT); // Assigning pin to OUTPUT mode
  prev_x = 0;

  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);

  lcd.init();
  lcd.clear();         
  lcd.backlight();

  pinMode(servoPin, OUTPUT);

  pinMode(motorPin, OUTPUT);

  servo.attach(servoPin); 
  servo.write(0);
}

void openServo() {
  if(servo.read() == 0) {
    for(int angle = 0; angle <= 180; angle++) {
        servo.write(angle);
        delay(1);
    }
  }
}

void closeServo() {
  if(servo.read() == 180) {
    for(int angle = 180; angle >= 0; angle--) {
        servo.write(angle);
        delay(1);
    }
  }
}

void writeMessageLCD(String message1, String message2) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(message1);
  lcd.setCursor(0,1);
  lcd.print(message2);
}
  
void loop() {
  float a;

  char message2[3];
  String text;
  char c;
  int k = 0;

  // read the message sent by bluetooth
  while(BTSerie.available()) {
    c = BTSerie.read();
    message2[k] = c;
    k++;
  }

  message2[k] = '\0';

  Serial.println("Message " + String(message2));

  if(message2[0] == '-') // start the fan
    ok = 1;
  else if (message2[0] != 'o' && strlen(message2) != 0) {
    ok = 0;
    total_people = atoi(message2);
  }

  Serial.println("Ok: " + String(ok) + " dangerFlame: " + String(dangerFlame));

  if(ok == 1 && dangerFlame == 0 && dangerEarthquake == 0 && dangerGas == 0)
    analogWrite(motorPin, 150);

  if(ok == 0 || dangerFlame == 1 || dangerEarthquake == 1 || dangerGas == 1) 
    analogWrite(motorPin, 0);

  // open door (if we receive 'o')
  if(message2[0] == 'o') { 
    openServo();
  }

  // close door (if we receive a number)
  if(message2[0] != '-' && message2[0] != 'o' && strlen(message2) != 0) {
    closeServo();
  }

  // temperature sensor
  int reading = analogRead(TempSensor);
  float voltage = reading * (5.0 / 1024.0);
  float temperatureC = voltage * 100;

  if(message2[0] != '-' && dangerFlame == 0 && dangerEarthquake == 0 && dangerGas == 0) {
    writeMessageLCD("People today: " + String(total_people), "Temp: " + String(temperatureC));
    Serial.println("People: " + String(total_people));
  }

  // flame sensor
  int sensorValue = analogRead(A1);
  Serial.println(sensorValue);
  if (sensorValue > 200) {
    dangerFlame = 1;
    fireTime = millis();
    openServo();
    tone(buzzerPin, 1000);
    //writeMessageLCD("FIRE DANGER", "LEAVE ROOM");
  }
  else if (sensorValue < 200 && message2[0] != '-' && message2[0] != 'o' && dangerEarthquake != 1 && dangerGas != 1) {
    dangerFlame = 0;
    noTone(buzzerPin);
    //writeMessageLCD("People today: " + String(total_people), "Temp: " + String(temperatureC));
  }

  if(dangerFlame == 1)
    writeMessageLCD("FIRE DANGER", "LEAVE ROOM");

  if(millis() - fireTime >= 1000 && fireTime != 0) {
    closeServo();
    fireTime = 0;
  }

  // shake sensor
  for (int i = 0; i < 10; i++) {
    val_x += analogRead(A_X);
    delay(2);
  }
  
  val_x = val_x / 10;
  delay(300);

  Serial.print(" X_Axis: ");
  Serial.print(val_x);
  Serial.print("      ");

  // Detect significant change in X-axis value
  if (prev_x != 0 && abs(val_x - prev_x) > X_THRESHOLD) {
    Serial.println("Earthquake Detected!");  // Trigger earthquake detection
    dangerEarthquake = 1;
    startMillis = millis();
    openServo();
    writeMessageLCD("EARTHQUAKE", "LEAVE ROOM");
    tone(buzzerPin, 1000); //Activate the alarm (buzzer); Generate a tone at a frequency of 50Hz
    //delay(50);
  }
  else if(millis() - startMillis >= 5000 && startMillis != 0) {
    dangerEarthquake = 0;
    startMillis = 0;
    closeServo();
    
    noTone(buzzerPin);
  }

  if(dangerEarthquake == 1) {
    writeMessageLCD("EARTHQUAKE", "LEAVE ROOM");
    tone(buzzerPin, 1000); //Activate the alarm (buzzer); Generate a tone at a frequency of 50Hz
  }

  prev_x = val_x;
  val_x = 0;

  // gas sensor
  int gas;
  gas = analogRead(A2);//Read Gas value from analog 0
  Serial.println(gas,DEC);//Print the value to serial port
  delay(100);

  if (gas >= 500) {
    dangerGas = 1;
    gasTime = millis();
    openServo();
    tone(buzzerPin, 1000);
    //writeMessageLCD("FIRE DANGER", "LEAVE ROOM");
  }
  else if (gas < 500 && message2[0] != '-' && message2[0] != 'o' && dangerEarthquake != 1 && dangerFlame != 1) {
    dangerGas = 0;
    noTone(buzzerPin);
    //writeMessageLCD("People today: " + String(total_people), "Temp: " + String(temperatureC));
  }

  if(dangerGas == 1)
    writeMessageLCD("GAS DANGER", "LEAVE ROOM");

  if(millis() - gasTime >= 1000 && gasTime != 0) {
    closeServo();
    gasTime = 0;
  }
 
}
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"


#include <ESP32Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <RevEng_PAJ7620.h>
#include  <WiFi.h>

#define DATABASE_URL "https://smartbin-galaksija-default-rtdb.firebaseio.com/"
#define WIFI_SSID "Galaxy A54"
#define WIFI_PASSWORD "12345678"
#define API_KEY "AIzaSyAuVIPXZ2hFkuvtv2vLURVdaTTajXJGOXg"

RevEng_PAJ7620 sensor = RevEng_PAJ7620();
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseJson json;
FirebaseConfig config;
bool signupOK = false;

int servoPin = 26;
int servosarka = 32;
Servo myservo;
Servo mysarka;

const int trigPin = 14;
const int echoPin = 12;
int pos = 0;
int prevmilis;
int status = 1;
long duration;
float distanceCm;

int Position ;

#define SOUND_SPEED 0.034
#define SS_PIN 17
#define RST_PIN 34
MFRC522 mfrc522(SS_PIN, RST_PIN);   
 
void setup() 
{
	Serial.begin(115200);

  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 
       Serial.println("SPI Begin");
  SPI.begin(18, 23, 19, 34);
  Serial.println("SPI Konekcija uspesna");
  mfrc522.PCD_Init();   
  Serial.println("Približite karticu čitaču");
  Serial.println();


  ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	myservo.setPeriodHertz(50);
	myservo.attach(servoPin, 500, 2400);
  mysarka.setPeriodHertz(50);
  mysarka.attach(servosarka, 500, 2400);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Prijava na WiFi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Konektovan sa IP adresom: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Firebase konekcija uspesna");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  } 
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Detekcija pokreta zapoceta\n");
    Serial.println("PAJ7620 sensor demo: Recognizing all 9 gestures.");

  if( !sensor.begin() )           // return value of 0 == success
  {
    Serial.print("PAJ7620 I2C error - halting");
    while(true) { }
  }

  Serial.println("PAJ7620 init: OK");
  Serial.println("Please input your gestures:");
  myservo.write(0);
  Serial.println("Kontejner dostupan");
  }

void loop() 
{
  uint8_t data = 0, data1 = 0, error;
	Gesture gesture;                  // Gesture is an enum type from RevEng_PAJ7620.h
  gesture = sensor.readGesture();				// Read Bank_0_Reg_0x43/0x44 for gesture result.
	if (status == 1) 
	{
		switch (gesture) 									// When different gestures be detected, the variable 'data' will be set to different values by paj7620ReadReg(0x43, 1, &data).
		{
			case GES_LEFT:
					Serial.println("Zatvoreno"); 
          myservo.write(90);              
          delay(500);  
				break;
			case GES_RIGHT: 
				Serial.println("Otvoreno");
            myservo.write(0);              
        delay(500); 
				break;
    }
  }
  else {
    Serial.println("Kontejner prenapunjen, molimo pokusajte kasnije");
  }
  if (millis() - prevmilis > 7000){
  checkbin();
  prevmilis = millis();
  }
	delay(100);
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Status : ");
  content.toUpperCase();
  if (content.substring(1) == "01 DB A6 26")
  {
    Serial.println("Pristup dozvoljen");
    myservo.write(0);              
    mysarka.write(0);              
    delay(100); 
    Serial.println();
    delay(10000);
    myservo.write(90);
  }
}

void checkbin(){
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED/2;
  Serial.print("Preostalo mesta (cm): ");
  Serial.println(distanceCm);

  if (distanceCm <= 10){
    status = 0;          
    Serial.println("motor");
		delay(500); 
  }
  else{
    status = 1;               
    Serial.println("motor");
		delay(500); 
  }
    if (Firebase.ready() && signupOK){
      if (Firebase.RTDB.setFloat(&fbdo, "data/indicator", distanceCm)){
      Serial.println("POSLATO");
      Serial.println("PUTANJA: " + fbdo.dataPath());
      Serial.println("TIP: " + fbdo.dataType());
    }
    else {
      Serial.println("NEUSPESNO SLANJE");
      Serial.println("RAZLOG: " + fbdo.errorReason());
    }
    }
}



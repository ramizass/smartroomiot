#include "DHT.h" //DHT library 
#include <Servo.h> //servo library 

#include <LiquidCrystal_I2C.h> //lcd library
LiquidCrystal_I2C lcd(0x27, 16, 2); //0x27 -> address bus; 16x2: coloumns x rows

#define FAN_PIN D3        //relay pin
#define flamePin D5        //flame pin
#define mqPin A0          //mq2 pin
#define DHTPIN D4         //DHT pin
#define DHTTYPE DHT22     //DHT type
DHT dht(DHTPIN, DHTTYPE); //make an object for dht with param: dhtpin and dhttype

Servo siservo; //make a siservo object from func Servo
const int buzz = D6; //buzzer constant with int data type

// wifi
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include <addons/RTDBHelper.h>

#define WIFI_SSID "YOUR SSID"
#define WIFI_PASSWORD "YOU WIFI PASSWORD"
#define API_KEY "YOUR DATABASE API"
#define DATABASE_URL "YOUR DATABASE URL"

//firebase object
FirebaseData fbdo; 
FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
bool signupOK = false; //set default condition

void setup() {
  Serial.begin(9600); //set baud rate
  dht.begin(); //set up dht
  siservo.attach(D7);  //connect the servo to pin 9  
  pinMode(buzz, OUTPUT); //set buzzer to output
  pinMode(FAN_PIN,OUTPUT); //set relay to output

  //set up wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("."); delay(300);
  } //loop until connected
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  config.api_key = API_KEY; //config the api
  config.database_url = DATABASE_URL; //config the url
  if(Firebase.signUp(&config, &auth, "", "")){ //signing the anonym user with objects without email n pass
    Serial.println("signUp OK");
    signupOK = true; 
  } else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str()); //error message
  }

  config.token_status_callback = tokenStatusCallback; //handle changing token
  Firebase.begin(&config, &auth); //set up firebase with objects
  Firebase.reconnectWiFi(true); //auto reconnect to wifi

  lcd.init(); //set up or initialization
  lcd.backlight();
  lcd.setCursor(6,0); lcd.print("Smart");
  lcd.setCursor(6,1); lcd.print("Room");
  delay(2000);
  lcd.clear();
}

void loop() {
  //read the data from sensor and save it to variable
  int bacaApi = digitalRead(flamePin);
  int gasState = analogRead(mqPin);
  float suhu = dht.readTemperature(); //calling method from dht library 
  float kelembapan = dht.readHumidity(); 

  siservo.write(0); //set the default state
  
  //set up the condition
  if (suhu > 32 && gasState < 600 && bacaApi == HIGH) { //temperature
    digitalWrite(FAN_PIN, HIGH); 
    siservo.write(180);
    noTone(buzz);

} else if (suhu > 32 && gasState > 600 && bacaApi == LOW) { //temperature, gas, and flame
    digitalWrite(FAN_PIN, LOW);
    siservo.write(180);
    tone(buzz, 250);

} else if (suhu > 32 && gasState > 600 && bacaApi == HIGH) { //temperature and gas
    digitalWrite(FAN_PIN, HIGH);
    siservo.write(180);
    noTone(buzz);

} else if (suhu < 32 && gasState > 600 && bacaApi == HIGH) { //gas 
    digitalWrite(FAN_PIN, HIGH);
    siservo.write(180);
    noTone(buzz);

} else if (suhu < 32 && gasState < 600 && bacaApi == LOW) { //flame
    digitalWrite(FAN_PIN, LOW);
    siservo.write(180);
    tone(buzz, 250);

} else { //nothing
    digitalWrite(FAN_PIN, LOW);
    siservo.write(0);
    noTone(buzz);
}
  
  //calling the function
  sens_dht22(suhu, kelembapan);
  sens_mq2(gasState);
  sens_Flame(bacaApi);
  
}

void sens_dht22(float suhu, float kelembapan){
  if (isnan(kelembapan) || isnan(suhu)) { //if the data not a number
    Serial.println("Suhu dan kelembaban tidak terbaca!");
    return;
  }
  Serial.println("Output DHT22 :");
  Serial.print("Humidity: "); Serial.print(kelembapan); Serial.println(" %\t");
  Serial.print("Temperature: "); Serial.print(suhu); Serial.println(" *C \n");

  lcd.setCursor(0, 0); lcd.print("Suhu :"); 
  lcd.setCursor(7, 0); lcd.print(suhu); 
  lcd.setCursor(0, 1); lcd.print("Humidity:"); 
  lcd.setCursor(9, 1); lcd.print(kelembapan);

  if(Firebase.RTDB.setFloat(&fbdo, "Sensor/Suhu", suhu)){ //setFloat func for the objects
      Serial.print(suhu);
      Serial.print(" - successfully saved to: " + fbdo.dataPath()); //get the path
      Serial.println(" (" + fbdo.dataType() + ")"); //get the data type
    } else{
      Serial.println("Failed: " + fbdo.errorReason()); //error message
    }

    if(Firebase.RTDB.setFloat(&fbdo, "Sensor/Kelembapan", kelembapan)){
      Serial.print(kelembapan);
      Serial.print(" - successfully saved to: " + fbdo.dataPath());
      Serial.println(" (" + fbdo.dataType() + ")");
    } else{
      Serial.println("Failed: " + fbdo.errorReason());
    }

  delay(2000); lcd.clear();
}

void sens_mq2(int gasState){
  Serial.print("Output MQ-2 : ");
    lcd.setCursor(0, 0); lcd.print("Output MQ2 :"); 
    lcd.setCursor(12, 0); lcd.print(gasState);
    
  Serial.println(gasState);
  if(gasState > 600){
    Serial.println("Gas detected\n");
    lcd.setCursor(0, 1); lcd.print("Udara Kotor");
  }
  else{
    Serial.println("Gas NOT detected\n");
    lcd.setCursor(0, 1); lcd.print("Udara Bersih");
  }

  if(Firebase.RTDB.setFloat(&fbdo, "Sensor/Indeks Gas", gasState)){
      Serial.print(gasState);
      Serial.print(" - successfully saved to: " + fbdo.dataPath());
      Serial.println(" (" + fbdo.dataType() + ")");
  } else{
      Serial.println("Failed: " + fbdo.errorReason());
    }

  delay(2000); lcd.clear();
}

void sens_Flame(int api){
  String strApi = ""; //blank variable for "YES" or "NO"
  Serial.println("Output Sensor Api :");
  lcd.setCursor(0, 0); lcd.print("Pendeteksi Api :"); 
  if (api == LOW) {
    strApi = "YES";
    Serial.println("Flame detected\n");
    lcd.setCursor(0, 1); lcd.print("Ada APi!!!!!");
  } else {
    strApi = "NO";
    Serial.println("No flame detected\n");
    lcd.setCursor(0, 1); lcd.print("Tidak Ada Api");
  }

  if(Firebase.RTDB.setString(&fbdo, "Sensor/Deteksi Api", strApi)){
      Serial.print(strApi);
      Serial.print(" - successfully saved to: " + fbdo.dataPath());
      Serial.println(" (" + fbdo.dataType() + ")");
  } else{
      Serial.println("Failed: " + fbdo.errorReason());
    }
  delay(2000); lcd.clear();
}

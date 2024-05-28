#include "DHT.h"
#include <Servo.h>

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define FAN_PIN D3
#define flamePin D5        // Deklarasi pin flame
#define mqPin A0          // Deklarasi pin mq2
#define DHTPIN D4         // Deklarasi pin DHT
#define DHTTYPE DHT22     // Tipe sensor DHT
DHT dht(DHTPIN, DHTTYPE); // Inisialisasi sensor DHT

Servo siservo;
const int buzz = D6;

// wifi
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include <addons/RTDBHelper.h>

#define WIFI_SSID "Cremona Lt 2"
#define WIFI_PASSWORD "12345678"
#define API_KEY "AIzaSyDa9mqpSKBWO7kyJ_Jw9op4x8c9jd88RGg"
#define DATABASE_URL "https://smart-room-64cf8-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;


unsigned long dataMillis = 0;
bool signupOK = false;

void setup() {
  Serial.begin(9600);
  dht.begin(); 
  siservo.attach(D7);  // Connect the servo to pin 2  
  pinMode(buzz, OUTPUT);
  relay1:pinMode(FAN_PIN,OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("."); delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if(Firebase.signUp(&config, &auth, "", "")){
    Serial.println("signUp OK");
    signupOK = true;
  } else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true); 

  lcd.init();
  lcd.backlight();
  lcd.setCursor(6,0); lcd.print("Smart");
  lcd.setCursor(6,1); lcd.print("Room");
  delay(2000);
  lcd.clear();
}

void loop() {
  int bacaApi = digitalRead(flamePin);
  int gasState = analogRead(mqPin);
  float suhu = dht.readTemperature();     // Baca suhu
  float kelembapan = dht.readHumidity(); 

  siservo.write(0);
  
  //Mengatur kondisi kipas dan servo berdasarkan suhu, gas, dan sensor flame
  if (suhu > 32 && gasState < 600 && bacaApi == HIGH) { 
    digitalWrite(FAN_PIN, HIGH); 
    siservo.write(180);
    noTone(buzz);

} else if (suhu > 32 && gasState > 600 && bacaApi == LOW) {
    digitalWrite(FAN_PIN, LOW);
    siservo.write(180);
    tone(buzz, 250);

} else if (suhu > 32 && gasState > 600 && bacaApi == HIGH) {
    digitalWrite(FAN_PIN, HIGH);
    siservo.write(180);
    noTone(buzz);

} else if (suhu < 32 && gasState > 600 && bacaApi == HIGH) {
    digitalWrite(FAN_PIN, HIGH);
    siservo.write(180);
    noTone(buzz);

} else if (suhu < 32 && gasState < 600 && bacaApi == LOW) { // Tambahkan kondisi suhu dan gasState
    digitalWrite(FAN_PIN, LOW);
    siservo.write(0);
    noTone(buzz); // Mematikan buzzer saat api tidak terdeteksi

} else if ( bacaApi == LOW) { // Kondisi api terdeteksi, tapi suhu dan gasState diabaikan
    digitalWrite(FAN_PIN, HIGH);
    tone(buzz, 250);
    siservo.write(180);

} else { // Kondisi default
    digitalWrite(FAN_PIN, LOW);
    siservo.write(0);
    noTone(buzz);
}

  // else if (suhu <30 && gasState < 600 && bacaApi == LOW){ //konfisi testing doang
  //   digitalWrite(FAN_PIN, HIGH);
  //   siservo.write(180);
  //   tone(buzz, 250);
  // } 
  
  sens_dht22(suhu, kelembapan);
  sens_mq2(gasState);
  sens_Flame(bacaApi);
  
}

void sens_dht22(float suhu, float kelembapan){
  if (isnan(kelembapan) || isnan(suhu)) {
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

  if(Firebase.RTDB.setFloat(&fbdo, "Sensor/Suhu", suhu)){
      Serial.print(suhu);
      Serial.print(" - successfully saved to: " + fbdo.dataPath());
      Serial.println(" (" + fbdo.dataType() + ")");
    } else{
      Serial.println("Failed: " + fbdo.errorReason());
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
  if(gasState > 580){
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
  String strApi = "";
  Serial.println("Output Sensor Api :");
  lcd.setCursor(0, 0); lcd.print("Pendeteksi Api :"); 
  if (api == LOW) {             // Cek apakah flame terdeteksi
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
#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <LiquidCrystal_I2C.h>

//-----------------------------------------
#define RST_PIN  D3
#define SS_PIN   D4
#define BUZZER   D8
//-----------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;  
MFRC522::StatusCode status;      

int blockNum = 2;  
byte bufferLen = 18;
byte readBlockData[18];

String card_holder_name;

const String sheet_url = "https://script.google.com/macros/s/AKfycbylWm_DF01JKfWH0kEIY5OZ1CzOcO-k3PqnTy_zlJnPOf7iKFhwJJO7HyojFGTNqYqBwQ/exec?name="; // put your Web App URL

#define WIFI_SSID "POCO M2 Pro"
#define WIFI_PASSWORD "Abir1234567890"

// LCD I2C Address
LiquidCrystal_I2C lcd(0x27, 16, 2);

//--------------------------------------------------------------
void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Initializing  ");
  delay(1000);

  Serial.println();
  Serial.print("Connecting to WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int wifi_attempt = 0;
  while (WiFi.status() != WL_CONNECTED && wifi_attempt < 50) {
    Serial.print(".");
    delay(200);
    wifi_attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(2000);
  } else {
    Serial.println("\nWiFi Failed");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed");
    delay(2000);
  }

  pinMode(BUZZER, OUTPUT);

  SPI.begin();
  mfrc522.PCD_Init();
  delay(1000);
  Serial.println("Ready to scan...");
}

//--------------------------------------------------------------
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int wifi_attempt = 0;
    while (WiFi.status() != WL_CONNECTED && wifi_attempt < 50) {
      delay(200);
      wifi_attempt++;
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi Reconnected.");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WiFi Reconnected");
      delay(2000);
    } else {
      Serial.println("\nWiFi Failed Again");
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Scan your Card ");

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println();
  Serial.println("Reading RFID...");
  ReadDataFromBlock(blockNum, readBlockData);

  Serial.println();
  Serial.print("RFID Data: ");
  Serial.write(readBlockData, 16);
  Serial.println();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hey ");
  lcd.print(String((char*)readBlockData));
  lcd.setCursor(0, 1);
  lcd.print("Welcome!");

  digitalWrite(BUZZER, HIGH); delay(200);
  digitalWrite(BUZZER, LOW); delay(200);
  digitalWrite(BUZZER, HIGH); delay(200);
  digitalWrite(BUZZER, LOW);

  // Send to Google Sheet
  if (WiFi.status() == WL_CONNECTED) {
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();

    card_holder_name = sheet_url + String((char*)readBlockData);
    card_holder_name.trim();
    Serial.println(card_holder_name);

    HTTPClient https;
    Serial.println("[HTTPS] begin...");

    if (https.begin(*client, card_holder_name)) {
      Serial.println("[HTTPS] GET...");
      int httpCode = https.GET();

      if (httpCode > 0) {
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        lcd.setCursor(0, 1);
        lcd.print(" Data Recorded ");
        delay(2000);
      } else {
        Serial.printf("[HTTPS] GET failed: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
      delay(1000);
    } else {
      Serial.println("[HTTPS] Unable to connect");
    }
  } else {
    Serial.println("WiFi not connected");
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(3000);
}

//--------------------------------------------------------------
void ReadDataFromBlock(int blockNum, byte readBlockData[]) {
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Auth failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Read failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  Serial.println("Block read OK");
}


#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include <MFRC522.h>

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;
#define SS_PIN D4
#define RST_PIN D8

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
Serial.begin(115200);
Wire.begin(D2, D1);
SPI.begin();      //Inicia  SPI bus
mfrc522.PCD_Init();   //Inicia MFRC522 
//Prepara chave - padrao de fabrica = FFFFFFFFFFFFh
for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
lcd.begin();
//Conecta ao WIFI
Serial.println("iniciando");

  Serial.println();
  Serial.println();
  Serial.println();
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
WiFi.mode(WIFI_STA);
WiFiMulti.addAP("Futurum_B", "Futurum#CRM");
}
void request(String id){
 lcd.clear();
  lcd.home();
  lcd.print("Autenticando...");
if ((WiFiMulti.run() == WL_CONNECTED)) {
    WiFiClient client;
    HTTPClient http;
    if (http.begin(client, "http://quimictec.herokuapp.com/api/catraca/registra/"+id)){
      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
           if(payload == "sucesso"){
          Serial.println("Autorizado");
          lcd.clear();
          lcd.home();
          lcd.print("Autorizado");
          delay(3000);
          }else if(payload == "falha"){
          lcd.home();
          lcd.print("Nao autorizado");
          delay(3000);
          Serial.println("Não autorizado");
          } 
        }
      } else {
        Serial.printf("[HTTP] falha: %s\n", http.errorToString(httpCode).c_str());
        lcd.clear();
        lcd.home();
        lcd.print("Sem internet");
      }
      http.end();
    }
  }    
}
void loop() { 
 if ((WiFiMulti.run() == WL_CONNECTED)) {
 modo_leitura();
 }
}

void modo_leitura(){
  lcd.clear();
  lcd.home();
  lcd.print("Encoste o card");
  //Aguarda cartao
    while ( ! mfrc522.PICC_IsNewCardPresent())
  {
    delay(100);
  }
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  String conteudo = "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  byte piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    //Obtem os dados do setor 1, bloco 4 = Nome
  byte sector         = 1;
  byte blockAddr      = 4;
  byte trailerBlock   = 7;
  byte status;
  byte buffer[18];
  byte size = sizeof(buffer);
 
  //Autenticacao usando chave A
  status=mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                  trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    
    return;
  }
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    
  }
  //Mostra os dados do nome no Serial Monitor
  for (byte i = 1; i < 16; i++)
  {
    //Serial.print(char(buffer[i]));
    lcd.print(char(buffer[i+1]));
  }
  
  
  Serial.println();
 
  //Obtem os dados do setor 0, bloco 1 = ID
  sector         = 0;
  blockAddr      = 1;
  trailerBlock   = 3;
 
  //Autenticacao usando chave A
  status=mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                  trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("PCD_Authenticate() failed: "));
    
    return;
  }
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("MIFARE_Read() failed: "));
    
  }
  //Mostra os dados do ID no Serial Monitor 

  //Serial.print("ID: ");
  String id = "";
  for (byte i = 0; i < 16; i++)
  {
    id.concat(char(buffer[i]));
    id.trim();
  }
  Serial.println(id);
  request(id);

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1(); 
}

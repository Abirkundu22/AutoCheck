#include <SPI.h>
#include <MFRC522.h>

//-------------------- Pins for NodeMCU ----------------------
constexpr uint8_t RST_PIN = D3;
constexpr uint8_t SS_PIN = D4;
//------------------------------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

//------------------------------------------------------------
// Set the block to write data
int blockNum = 2;

// The data to write into RFID Tag
byte blockData[16] = {"Bubai"};  // Change the name you want to store

// Create array to read data from Block
byte bufferLen = 18;
byte readBlockData[18];

MFRC522::StatusCode status;

//------------------------------------------------------------

void setup() 
{
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  // ?? Important: Slow down SPI clock to 1 MHz for ESP8266
  SPI.setFrequency(1000000);

  Serial.println("Scan a RFID Tag to write data...");
}

//------------------------------------------------------------
void loop()
{
  // Prepare the key for authentication
  for (byte i = 0; i < 6; i++){
    key.keyByte[i] = 0xFF;
  }

  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println();
  Serial.println("*Card Detected*");

  // Print UID
  Serial.print(F("Card UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++){
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Print Card Type
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // Write to Block
  Serial.println();
  Serial.println("Writing to Data Block...");
  WriteDataToBlock(blockNum, blockData);

  // Read from Block
  Serial.println();
  Serial.println("Reading from Data Block...");
  ReadDataFromBlock(blockNum, readBlockData);

  // Print the data read
  Serial.println();
  Serial.print("Data in Block:");
  Serial.print(blockNum);
  Serial.print(" --> ");
  for (int j = 0; j < 16; j++){
    Serial.write(readBlockData[j]);
  }
  Serial.println();
}

//------------------------------------------------------------
void WriteDataToBlock(int blockNum, byte blockData[]) 
{
  status = mfrc522.PCD_Authenticate(
              MFRC522::PICC_CMD_MF_AUTH_KEY_A,
              blockNum, 
              &key, 
              &(mfrc522.uid)
           );

  if (status != MFRC522::STATUS_OK){
    Serial.print("Authentication failed for Write: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else {
    Serial.println("Authentication success");
  }

  status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Writing to Block failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else {
    Serial.println("Data was written into Block successfully");
  }
}

//------------------------------------------------------------
void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{
  status = mfrc522.PCD_Authenticate(
              MFRC522::PICC_CMD_MF_AUTH_KEY_A,
              blockNum, 
              &key, 
              &(mfrc522.uid)
           );

  if (status != MFRC522::STATUS_OK){
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else {
    Serial.println("Authentication success");
  }

  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK){
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else {
    Serial.println("Block was read successfully");
  }
}

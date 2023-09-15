#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>            
#include <wiring_private.h>
#include <math.h>
#define FILE_WRITE (O_READ | O_WRITE | O_CREAT | O_APPEND)
#define SS_PIN 10 //RFID digital PIN
#define RST_PIN 9  //RFID reset PIN
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define NUMFLAKES     10 // Number of snowflakes in the animation example
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
//#define PIN_SERIAL2_RX       (2ul)              //    Pin description number for PIO_SERCOM on D1
//#define PIN_SERIAL2_TX       (3ul)             //     Pin description number for PIO_SERCOM on D0
//#define PAD_SERIAL2_TX       (UART_TX_PAD_0)    //    SERCOM pad 2 TX
//#define PAD_SERIAL2_RX       (SERCOM_RX_PAD_1)   //   SERCOM pad 3 RX

static const unsigned char PROGMEM logo_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key; 

Uart Lora(&sercom1, 3, 2,SERCOM_RX_PAD_1,UART_TX_PAD_0);  // Communication with Household
void SERCOM1_Handler()   //   Interrupt handler for SERCOM1
{  Lora.IrqHandler();}

char ch[32] = {0};
String credit = "500";
char c_array[10] = {0};
String id = "";
const int chipSelect = 8;
// Init array that will store new NUID 
byte nuidPICC[4];
String ID;
int i=0;
String cardID="1B 3E 1B 6D";
bool idMatch =true;


void setup() { 
  Serial.begin(9600);
  Lora.begin(4800);
  pinPeripheral(3, PIO_SERCOM_ALT);  //   Assign pins 0 & 1 SERCOM functionality
  pinPeripheral(2, PIO_SERCOM_ALT);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }  

}
 
void loop() 
{

    Lora.print("Howon");
    Serial.println("sent");
    delay(2000);

  Oled.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setCursor(0,0);
  display.println("Please tag your card");
  display.display();
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
  {
    readCardID();
//  Request info to the Hope solar plant
//  Recieve info from the Hope solar plant
//  valve control
//  This will be done at day 2
//  }
}
}

/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void readCardID()
{
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }
  Serial.println(F("The NUID tag is:"));
  Serial.print(F("In hex: "));
  String tag_id = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    tag_id.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    tag_id.concat(String(rfid.uid.uidByte[i], HEX));
  }
  tag_id.toUpperCase();
  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
  ID = tag_id.substring(1);
  Serial.println(ID);

}

void valve_control()
{
  digitalWrite(7,HIGH);
  delay(20000);
  digitalWrite(7,LOW);
}
  /*
//-------------UPDATE INFO----------------

  for(int i=teststring2.length(); i<15; i++)
  {
    teststring2 +=' ';
  }
  
  String modi = teststring0 + comma+teststring1 + comma + teststring2;
  myFile.seek(positionSD); //2번째줄 위치
  myFile.print(modi); //2번째줄 위치 업데이트
  Serial.println("-----------After change---------------");
  myFile.seek(0);
  line = myFile.readStringUntil('\0'); //read line one by one
  Serial.println(line);

}*/

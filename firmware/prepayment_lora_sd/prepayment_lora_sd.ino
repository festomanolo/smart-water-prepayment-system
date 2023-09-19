#include <Arduino.h>            
#include <wiring_private.h>
#include <math.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//SD card mode change
#define FILE_WRITE (O_READ | O_WRITE | O_CREAT | O_APPEND)
//RFID
#define SS_PIN 10
#define RST_PIN 9
//OLED configuration
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define NUMFLAKES     10 // Number of snowflakes in the animation example
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Uart LoraSerial(&sercom0, 5, 6,SERCOM_RX_PAD_1,UART_TX_PAD_0);  // Communication with Household
void SERCOM0_Handler()   //   Interrupt handler for SERCOM1
{  LoraSerial.IrqHandler();}

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
//File myFile;
//const int chipSelect = ; //SD card --> we are not using SD card for this project
const int flow_sensor = 8; //Flow sensor
const int valve = 7; //valve control through relay
char *strings[6]; // an array of pointers to the pieces of the above array after strtok()
char *ptr = NULL;
char ch[32] = {0};
char c_array[10] = {0};
String credit = "500";
String id = "";
// Init array that will store new NUID 
byte nuidPICC[4];
String ID;
int i=0;
bool idMatch =true;


void setup() { 
  pinPeripheral(5, PIO_SERCOM_ALT);  //   Assign pins 5 & 6 SERCOM functionality
  pinPeripheral(6, PIO_SERCOM_ALT);
  LoraSerial.begin(4800); //LoRa Serial2 start 
  Serial.begin(9600);
  //while(!Serial){;} //Uncomment for Serial communication
  pinMode(valve, OUTPUT);
  digitalWrite(valve, LOW);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
  display.drawPixel(10, 10, SSD1306_WHITE);
  display.display();
  delay(2000);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
//  if (!SD.begin(chipSelect)) {
//    Serial.println("Card failed, or not present");
//  }
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("IDIM"));
  display.println(F("Prepayment"));
  display.println(F("System"));
  display.display();
  delay(3000);
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Plea32156se Tag your Card"));
  display.display();
  delay(3000);
  

}
 
void loop() 
{
  if (LoraSerial.available()>0)
  {
    ID_identification();
  }

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
  {
    readCardID();
// ------------SD card mode----------- 
//    int userinfo = readSD_test(ID);
//    Serial.println(userinfo);   
//    if(userinfo>0)
//    {
//      SD_update(userinfo);
//    }
//------------------------------------
  }
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Please Tag your Card"));
  display.display();
}



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

  display.clearDisplay();
  display.setTextSize(1);             
  display.setTextColor(SSD1306_WHITE);       
  display.setCursor(0,0);             
  display.println(ID);
  display.display();
  LoraSerial.print(ID);
  delay(1000);
  Serial.println(ID);
  LoraSerial.begin(4800);

}


/* SD CARD MODE
int readSD_test(String ID)
{ 
  myFile = SD.open("test.txt", O_RDWR);
  myFile.seek(0);
  String line = myFile.readStringUntil('\0'); //read line one by one
  Serial.println(line);
  myFile.seek(0);
  Serial.println("----------------Searching--------------");
  idMatch =true;
  int positionSD =0;
  int prev_position=0;
  
  while(idMatch =true)
  {
    prev_position=positionSD;
    //line read and extract info as String index
    String line = myFile.readStringUntil('\n'); 
    delay(10);
    Serial.println(line.length());
    positionSD = myFile.position();
    line.toCharArray(ch,line.length()); 
    byte index = 0;
    ptr = strtok(ch, ",");
    while (ptr != NULL)
    {
       strings[index] = ptr;
       index++;
       ptr = strtok(NULL, ",");
    }
    String string0 = strings[0]; 
    String string1 = strings[1];
  

    //compare current card data with saved data in SD
    if(string0==ID)
    {
      Serial.print("Hello, ");
      Serial.println(string1);
      idMatch=false;
  
      return prev_position;
    }
    else if(prev_position == positionSD)
    {
      idMatch=false;
      Serial.println("We can not find your Card info");

      return 0;
    }
         


  }
}

void SD_update(int userinfo)
{
  myFile.seek(userinfo);
  String line = myFile.readStringUntil('\n'); 
  Serial.println(line);
  int   positionSD = myFile.position();
  line.toCharArray(ch,line.length()); 
  byte index = 0;
  ptr = strtok(ch, ",");
  while (ptr != NULL)
  {
     strings[index] = ptr;
     index++;
     ptr = strtok(NULL, ",");
  }
  
  String user_ID = strings[0]; 
  String user_name = strings[1];
  String user_credit = strings[2];
  String comma = ",";
  int credit = user_credit.toInt();
  if (credit < 10)
  {
    Serial.println("you don't have enough credit");
  }
  else
  {
    credit = credit-50;
    user_credit=String(credit);
    String modi = user_ID + comma+user_name + comma + user_credit;
    Serial.print(modi);
    for(int i=modi.length(); i<50; i++)
    {
      modi +=' ';
    }
    
    myFile.seek(userinfo);
    myFile.print(modi);
    myFile.seek(0);
    String line = myFile.readStringUntil('\0'); 
    valve_control();

    
    }
}*/
void ID_identification()
{
  String input = LoraSerial.readString(); // O;Name,credit/
  if (input[0] == 'N'){
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.println("Your card is not registered");
    display.display();
  }
  if (input[0] == 'O'){
    int locat1=input.indexOf(';');
    int locat2=input.indexOf(',');
    int locat3=input.indexOf('/');
    String userName = input.substring(locat1+1, locat2);
    String credit = input.substring(locat2+1, locat3);
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.print("Hello ");
    display.println(userName);
    display.print("Credit left: ");
    display.println(credit);
    display.display();
    valve_control();
  }
  if (input[0] == 'C'){
    int locat1=input.indexOf(';');
    int locat2=input.indexOf(',');    
    int locat3=input.indexOf('/');
    String userName = input.substring(locat1+1, locat2);
    String credit = input.substring(locat2+1, locat3);
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.print("Hello ");
    display.println(userName);
    display.print("Your credit is not enough, ");
    display.println(credit);
    display.display();  
    }
    LoraSerial.begin(4800);
}
void valve_control()
{
  digitalWrite(valve,HIGH);
  delay(10000);
  digitalWrite(valve,LOW);
  delay(10000);
  LoraSerial.begin(4800);
  
}
 

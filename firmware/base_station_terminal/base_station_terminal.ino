#include <SPI.h>
#include <Arduino.h>            
#include <wiring_private.h>
#include <math.h>
//#include "Sodaq_wdt.h"
//#ifdef ARDUINO_ARCH_AVR
//#define DIAG_STREAM Serial
//#elif ARDUINO_ARCH_SAMD
//#define DIAG_STREAM SerialUSB
//#endif
//C:\ProgramData\Microsoft\Windows\Start Menu\Programs\StartUp
//  Serial2 pin and pad definitions (in Arduino files Variant.h & Variant.cpp)
#define PIN_SERIAL2_RX       (1ul)              //    Pin description number for PIO_SERCOM on D1
#define PIN_SERIAL2_TX       (0ul)             //     Pin description number for PIO_SERCOM on D0
#define PAD_SERIAL2_TX       (UART_TX_PAD_0)    //    SERCOM pad 0 TX
#define PAD_SERIAL2_RX       (SERCOM_RX_PAD_1)   //   SERCOM pad 1 RX

//  Serial3 pin and pad definitions (in Arduino files Variant.h & Variant.cpp)
#define PIN_SERIAL3_RX       (5ul)             //     Pin description number for PIO_SERCOM on D5
#define PIN_SERIAL3_TX       (4ul)             //     Pin description number for PIO_SERCOM on D4
#define PAD_SERIAL3_TX       (UART_TX_PAD_2)     //   SERCOM pad 4 TX
#define PAD_SERIAL3_RX       (SERCOM_RX_PAD_3)    //  SERCOM pad 5 RX

 // Instantiate the extra Serial classes
Uart Serial2(&sercom3, PIN_SERIAL2_RX, PIN_SERIAL2_TX, PAD_SERIAL2_RX, PAD_SERIAL2_TX);  // Communication with Household
Uart Serial3(&sercom4, PIN_SERIAL3_RX, PIN_SERIAL3_TX, PAD_SERIAL3_RX, PAD_SERIAL3_TX);  // Communication with Station

void SERCOM3_Handler()   //   Interrupt handler for SERCOM3
{  Serial2.IrqHandler();}
void SERCOM4_Handler()   //   Interrupt handler for SERCOM4
{  Serial3.IrqHandler();}//  Initialize variables
int current; 
int voltage;
unsigned long timegap;
unsigned long request_time;
unsigned long cycle_time;
byte request = 0;
int home_num = 1;
int channels = 44;  // number of connected smart meters
                  // Friendship: (48), Hope: (49), Mkalama: (52)
boolean cycle = 1;
//cycle time controller
#define WAIT_METER 5000  // Signal waiting through smartmeter (ms)
#define WAIT_STATION 10000  // Signal waiting through station (ms)
#define WAIT_CYCLE 2000  // Delay between cycle (ms)

void setup(){
  Serial.begin(9600);
  Serial2.begin(4800);
  Serial3.begin(4800);
  //giving Serial communication functionality to pin 0,1,4,5
  pinPeripheral(0, PIO_SERCOM);  //   Assign pins 0 & 1 SERCOM functionality
  pinPeripheral(1, PIO_SERCOM);
  pinPeripheral(4, PIO_SERCOM_ALT);  //   Assign pins 0 & 1 SERCOM functionality
  pinPeripheral(5, PIO_SERCOM_ALT);
  
 // enable WATCHDOG TIMER - 8sec
//  sodaq_wdt_enable(WDT_PERIOD_8X);
  //DIAG_STREAM.begin(9600);
  delay(1000);
  cycle_time = millis();
}

void loop(){ 
//  sodaq_wdt_reset();          // uncomment to avoid reboot
  if(cycle){
  // Communication with Smart meter
    if(request==0){
      String com = "Q-";
      com += home_num;
      com += ";";
      Serial2.println(com);
      Serial.println(com);
      request = 1;
      request_time = millis();
    } // end of request==0
    
    if(request==1){
      if (Serial2.available()>0){  
        String input=Serial2.readString();
        Serial.print("get from smartmeter : ");
        Serial.println(input);
        int input_length=input.length();
        if (input_length == 13){  // "01;220,003/00"
    //      Serial.println(input);
    	 	  int locat1 = input.indexOf(';');
    	 	  int locat2 = input.indexOf(',');
          int locat3 = input.indexOf('/');
    			int homenum = input.substring(0, locat1).toInt();
          if(homenum==home_num){
            voltage = input.substring(locat1 + 1, locat2).toInt();
            current = input.substring(locat2 + 1, locat3).toInt();
            timegap = millis() - request_time;
      		  char msg[200];
            sprintf(msg,"D,%d,%d,%d,%d",homenum,timegap,voltage,current);
            Serial.println(msg);
            memset(msg,0,200);
            
            home_num += 1;
            Serial2.begin(4800);
            Serial3.begin(4800);
            request = 0;
          }
        } // end of if input_length==13
      } // end of serial2
      
      if((request==1) && (millis() - request_time > WAIT_METER)){  // Skip smartmeter if no-response during 2 minutes
        Serial.print(home_num);
        Serial.println(": Base station HELP!");
        request = 2;
      }
    }// end of request==1
    
  // Communication with Station
    if(request==2){  
      String com = "Q-";
      com += home_num;
      com += ";";
      Serial3.println(com);
      request = 3;
    }// end of request==2

    if(request==3){
      if (Serial3.available()>0){   
        String input=Serial3.readString();
        Serial.print("get from station : ");
        Serial.println(input);
        int input_length=input.length();
        if (input_length == 13){  // "01;220,003/00"
    //      Serial.println(input);
          int locat1 = input.indexOf(';');
          int locat2 = input.indexOf(',');
          int locat3 = input.indexOf('/');
          int homenum = input.substring(0, locat1).toInt();
          if(homenum==home_num){
            voltage = input.substring(locat1 + 1, locat2).toInt();
            current = input.substring(locat2 + 1, locat3).toInt();
            timegap = millis() - request_time;
            char msg[200];
            sprintf(msg,"D,%d,%d,%d,%d",homenum,timegap,voltage,current);
            Serial.println(msg);
            memset(msg,0,200);
            
            home_num += 1;
            request = 0;
            
            Serial2.begin(4800);
            Serial3.begin(4800);
          }
        } // end of if input_length==13
      } // end of serial3
      if((request==3) && (millis() - request_time > WAIT_STATION)){  // Skip smartmeter if no-response during 2 minutes
        Serial.print(home_num);
        Serial.println(": Skipped!");
        home_num += 1;
        Serial2.begin(4800);
        Serial3.begin(4800);
        request = 0;
      }
    }// end of request==3
  }

   // Wait cycle period
  if(home_num > channels){
    if(cycle){Serial.println("Cycle term wait");}
    cycle = 0;
    if((millis() - cycle_time) > WAIT_CYCLE){
      cycle = 1;
      home_num = 1;
      Serial.println("Start next cycle");
      cycle_time = millis();
    }
  }

// Communication with Serial
  if (Serial.available()>0){
    String input=Serial.readString();
    Serial.println(input);
    
    if(input[0] == 'T'){  // manual smartmeter control
      //Serial2.println(input);
      Serial3.println(input);
    }
    else if(input[0] == '#'){  // payment check signal
      //Serial2.println(input);
      Serial3.println(input);
    }
    else if(input.indexOf("u1013") != (-1)){
      Serial2.println(input);
      Serial3.println(input);
      home_num = 1;
      request = 0;
      delay(5000);
    }
    
  }
  
}

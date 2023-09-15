#include <SPI.h>
#include <Arduino.h>                       //         required before wiring_private.h
#include <wiring_private.h>
#include <math.h>
#include <SoftwareSerial.h>
//#include <avr/wdt.h>
// #include <EmonLib.h>
#include <EEPROM.h>
// EnergyMonitor emon1;
SoftwareSerial RF(2, 3);//Rx，tx  b
///////////////////device No: 01(십의자리,일의자리)/////////////////
// Write house number in order (ex 14 : Num_device_0 = 1 , Num_device_1 = 4)
int Num_device_0 = 1; 
int Num_device_1 = 4;



///////////////////////////////////////////////////////////////////////
int device_noo = 0;
int addr = 0;
int addr_LED = 1;
///////////////////senoring/////////////////////////////////////////
float total_current = 0;
float total_voltage = 0;
int count = 0;
int results[2][3];
//////////////////Calibration(1020)////////////////////////
float voltage = 0.;
long Vref_max = 0;      
long Vref_med = 0;  
long current_digit = 0;   
long current_origin = 0;    
//long noise_digit = 0;
float noise_rms = 0.08;  
int Vmax = 0;
float current = 0.;
int val1 = 0;
int val2 = 0;
///////////////////communication///////////////////////////////////
String previous_result = "";
String check_request = "Q-";
String check_reset = "R-";
String on_off_switch = "T-";
int ddiiss_ppllaayy = 0;
byte request = 0;
////////////////////relay////////////////////////////////////////////
int relay = 10; // RELAY
byte value = 0;
int security = 0;
int security_count = 0;

void setup()
{
	Serial.begin(9600);
	RF.begin(4800);
  pinMode(relay, OUTPUT);
  pinMode(A0, INPUT);//current
  pinMode(A1, INPUT);//voltage
  analogReference(EXTERNAL);
  digitalWrite(relay, LOW);
  
  Serial.println("Calibrating...");

  initializing(); 
  
}

void initializing(){
  
//display C
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  digitalWrite(4, HIGH);
  digitalWrite(5, LOW);
  digitalWrite(6, HIGH);
  digitalWrite(7, LOW);
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);
  digitalWrite(11, HIGH);
  digitalWrite(12, LOW);

  delay(5000);
  
  for(int i=0; i<1000; i++){                     // Current_origin calibration
    val1 = analogRead(A0);
    current_digit += val1;
  }
  current_origin = current_digit/1000;
  Serial.print("Current origin = ");
  Serial.println(current_origin);
  RF.print("Current origin = ");
  RF.println(current_origin);
  delay(1000);

//  for(int i=0; i<1000; i++){                     // Noise_rms calibration
//    val1 = analogRead(A0);
//    noise_digit += pow(val1-current_origin,2.0);
//  }
//  noise_rms = sqrt(noise_digit/1000.0)*30/sqrt(2)/1024;
//  Serial.print("Noise rms = ");
//  Serial.println(noise_rms);
//  RF.print("Noise rms = ");
//  RF.println(noise_rms);
//  delay(1000);

  for(int i=0; i<1000; i++){                     // Voltage calibration
    val2 = analogRead(A1);
    Vref_med += val2;
    if(val2>Vref_max){
      Vref_max = val2;
    }
  }
  Vref_med = Vref_med/1000;
  Serial.print("Voltage origin = ");
  Serial.println(Vref_med);
  RF.print("Voltage origin = ");
  RF.println(Vref_med);
  delay(1000);

  val1 = 0;
  val2 = 0;
  
  value = EEPROM.read(addr);
  digitalWrite(relay,value);
  Serial.print("initial state of Relay is " );
  Serial.println(value);
  
  ddiiss_ppllaayy=EEPROM.read(addr_LED);
  Serial.print("ddiiss_ppllaayy " );
  Serial.println(ddiiss_ppllaayy);

  feedback_led();
  
  device_noo=Num_device_0*10+Num_device_1;
  check_reset+=device_noo;
  check_reset+=";";
  Serial.print("check_reset: ");
  Serial.println(check_reset);
  on_off_switch+=device_noo;
  on_off_switch+="-";
  Serial.print("on_off_switch: ");
  Serial.println(on_off_switch);
  check_request+=device_noo;
  check_request+=";";
  Serial.print("check_request: ");
  Serial.println(check_request);
}

void communication()
{
	String input = "";
	String Input[2];
	int a = -1;
	int locat = 0;

	if (RF.available()>0){  //  RF signal come in
		input = RF.readString();
	  Serial.print("input:");
		Serial.println(input);
		int command_length = input.length();
		Serial.print("length: ");
		Serial.println(command_length);
   
		if ((input.indexOf(check_reset)!=(-1)) || (input.indexOf("u1013")!=(-1))){  // If reset command come in
		  Serial.println("previous_reseting");     
      Serial.print(previous_result);
      RF.println(previous_result);
      previous_result="";
			input = "";
			initializing();
		}
   /*  # Helping mode can be inserted here with "command_length == 13" condition 
    
		else if ((command_length == 13)&&(H==-1)&&(R==-1)) // 
		{ 
			locat = input.indexOf(';');
			Input[0]= input.substring(0, locat);
		  Input[1]= input.substring(locat + 1, input.length());
			help += Input[0]; help + = ",";// 
			
				if (helping_modeling ==1) 
				{
					if (for_help_No == Input[0].toInt()) 
					{
				//		String hp = Input[0]; hp += "\n"; hp += Input[1]; hp += "\n";
						Serial.print(input);  // helping data launch
						Serial.print(" IS HELPING ");  // test
						RF.print(input);  // test
					}
				}
		}
   */
		else if(input.indexOf(on_off_switch)!=(-1)){  // If on-off command come in
      String swittch=on_off_switch+"1";
      int zanshi=input.indexOf(swittch);
      if(zanshi==-1){
        EEPROM.write(addr, 0);
        digitalWrite(relay, 0);
        Serial.println("switch on");
      }
      else{
        EEPROM.write(addr, 1);
        digitalWrite(relay, 1);
        Serial.println("switch off");
      }
		}

    else if(input.indexOf(check_request)!=(-1)){  // If request command come in
      request = 1;
      Serial.println("Request is come in");
    }

    else if(input.indexOf("##")!=(-1)){  // If credit command come in
    	int payment_singal_checker= input.indexOf("##");
    	if (payment_singal_checker==0){
    	  Serial.println("Payment check");
    	  char c = input.charAt(2*(Num_device_0 * 10 + Num_device_1));
    	  ddiiss_ppllaayy = c;
    	  EEPROM.write(addr_LED, ddiiss_ppllaayy);
    	  Serial.print("Display label is ");
    	  Serial.println(ddiiss_ppllaayy);
        feedback_led();
    	}
    }

    else{
      Serial.println("Garbage input");
      input = "";
    }
    
	}// end of available
}// end of communication part

void sensing(){ 
  //initialize
  current_digit = 0;
  Vmax = 0;
  
  for(int i=0;i<1000;i++){
    val1 = analogRead(A0);  // current sensor
    current_digit += pow(val1 - current_origin,2.0);
  }
  total_current += sqrt(current_digit/1000.0)*30/512 - noise_rms;    // Compensating noise
  
  for(int i=0;i<1000;i++){
    val2 = analogRead(A1);  // voltage sensor
    if(val2>Vmax){Vmax = val2;}
  }
  total_voltage += float(Vmax-Vref_med)/float(Vref_max-Vref_med)*220.0;    // Assume test result is RMS 220V

  count += 1;

  Serial.println(total_voltage);
}

void feedback_led(){  //ASCII code로 구현
  if(ddiiss_ppllaayy==48){
    EEPROM.write(addr, 1);
    digitalWrite(relay, 1);
    
    digitalWrite(4,  HIGH);
    digitalWrite(5,  HIGH);
    digitalWrite(6,  HIGH);
    digitalWrite(7,  HIGH);
    digitalWrite(8,  LOW);
    digitalWrite(9,  HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(12, LOW);
  }
  else if(ddiiss_ppllaayy==49){
    EEPROM.write(addr, 0);
    digitalWrite(relay, 0);
    
    digitalWrite(4,  LOW);
    digitalWrite(5,  HIGH);
    digitalWrite(6,  LOW);
    digitalWrite(7,  HIGH);
    digitalWrite(8,  LOW);
    digitalWrite(9,  LOW);
    digitalWrite(11, LOW);
    digitalWrite(12, LOW);
  }
  else if(ddiiss_ppllaayy==50){
    EEPROM.write(addr, 0);
    digitalWrite(relay, 0);
    
    digitalWrite(4,  HIGH);
    digitalWrite(5,  HIGH);
    digitalWrite(6,  HIGH);
    digitalWrite(7,  LOW);
    digitalWrite(8,  LOW);
    digitalWrite(9,  LOW);
    digitalWrite(11, HIGH);
    digitalWrite(12, HIGH);
  }
  else if(ddiiss_ppllaayy==51){
    EEPROM.write(addr, 0);
    digitalWrite(relay, 0);
    
    digitalWrite(4,  HIGH);
    digitalWrite(5,  HIGH);
    digitalWrite(6,  HIGH);
    digitalWrite(7,  HIGH);
    digitalWrite(8,  LOW);
    digitalWrite(9,  LOW);
    digitalWrite(11, LOW);
    digitalWrite(12, HIGH);
  }
  else if(ddiiss_ppllaayy==52){
    EEPROM.write(addr, 0);
    digitalWrite(relay, 0);
    
    digitalWrite(4,  LOW);
    digitalWrite(5,  HIGH);
    digitalWrite(6,  LOW);
    digitalWrite(7,  HIGH);
    digitalWrite(8,  LOW);
    digitalWrite(9,  HIGH);
    digitalWrite(11, LOW);
    digitalWrite(12, HIGH);
  }
  else if(ddiiss_ppllaayy==53){
    EEPROM.write(addr, 0);
    digitalWrite(relay, 0);
    
    digitalWrite(4,  HIGH);
    digitalWrite(5,  LOW);
    digitalWrite(6,  HIGH);
    digitalWrite(7,  HIGH);
    digitalWrite(8,  LOW);
    digitalWrite(9,  HIGH);
    digitalWrite(11, LOW);
    digitalWrite(12, HIGH);
  }
  else if(ddiiss_ppllaayy==54){
    EEPROM.write(addr, 0);
    digitalWrite(relay, 0);
    
    digitalWrite(4,  HIGH);
    digitalWrite(5,  LOW);
    digitalWrite(6,  HIGH);
    digitalWrite(7,  HIGH);
    digitalWrite(8,  LOW);
    digitalWrite(9,  HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(12, HIGH);
  }
  else if(ddiiss_ppllaayy==55){
    EEPROM.write(addr, 0);
    digitalWrite(relay, 0);
    
    digitalWrite(4,  HIGH);
    digitalWrite(5,  HIGH);
    digitalWrite(6,  LOW);
    digitalWrite(7,  HIGH);
    digitalWrite(8,  LOW);
    digitalWrite(9,  HIGH);
    digitalWrite(11, LOW);
    digitalWrite(12, LOW);
  }
  else if(ddiiss_ppllaayy==56){
    EEPROM.write(addr, 0);
    digitalWrite(relay, 0);
    
    digitalWrite(4,  HIGH);
    digitalWrite(5,  HIGH);
    digitalWrite(6,  HIGH);
    digitalWrite(7,  HIGH);
    digitalWrite(8,  LOW);
    digitalWrite(9,  HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(12, HIGH);
  }  
  else if(ddiiss_ppllaayy==57){
    EEPROM.write(addr, 0);
    digitalWrite(relay, 0);
    
    digitalWrite(4,  HIGH);
    digitalWrite(5,  HIGH);
    digitalWrite(6,  HIGH);
    digitalWrite(7,  HIGH);
    digitalWrite(8,  LOW);
    digitalWrite(9,  HIGH);
    digitalWrite(11, LOW);
    digitalWrite(12, HIGH);
  }
  else if(ddiiss_ppllaayy==70 || ddiiss_ppllaayy==102){
    EEPROM.write(addr, 0);
    digitalWrite(relay, 0);
    
    digitalWrite(4,  HIGH);
    digitalWrite(5,  LOW);
    digitalWrite(6,  LOW);
    digitalWrite(7,  LOW);
    digitalWrite(8,  LOW);
    digitalWrite(9,  HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(12, HIGH);
  }
  else{
    digitalWrite(4,  HIGH);
    digitalWrite(5,  LOW);
    digitalWrite(6,  HIGH);
    digitalWrite(7,  LOW);
    digitalWrite(8,  HIGH);
    digitalWrite(9,  HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(12, HIGH);
  } 
}

void transmitting(){
  current = total_current / count;
  voltage = total_voltage / count;
  if(current < 0){current = 0;}
  if(voltage < 0){voltage = 0;}
  
  if(current>=4.5){  // Too much current!
    digitalWrite(relay,1);
    security=1;
  }

  // Data post-processing
  if(current<0){current = 0;}
  int current_first=(int) current;
  if(current_first<10){
    results[0][0]=current_first;
  }
  else{
    results[0][0]=0;
  }
  int current_second=(int)((current -current_first) *10);
  if(current_second<10){
    results[0][1]=current_second; 
  }
  else{
    results[0][1]=0;
  }
  int current_third=(int)( (current -current_first) *100-current_second*10);
  if(current_third<10){
    results[0][2]=current_third; 
  }
  else{
    results[0][2]=0;
  }
       
  if(voltage<1000){
    results[1][0]= (int) (voltage/100);
    results[1][1]= (int) ((voltage-results[1][0]*100)/10);
    results[1][2]= (int) (voltage-results[1][0]*100-results[1][1]*10);
  }
  else{
    results[1][0]= 0;
    results[1][1]= 0;
    results[1][2]= 0;
  }
  
  /*
  //helping model//
  if(helping_modeling==1){
    help_time++;
    if(help_time>=3){ //
      help_time=0;
      helping_modeling = 0;// helping_modeling = 0;//go back to the initial states
    }
  }
   //helping model//
   // Serial.print("help_mode: ");Serial.println(helping_modeling);//
    int location_3 = -1; int  ji = -1; int Help[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    do {
      location_3 = help.indexOf(',');
      if (location_3 != -1)
      {
        ji++;
        for (int i = 0; i<10; i++)
        {
          if (ji == i)
          {
            Help[ji] = help.substring(0, location_3).toInt();
            help = help.substring(location_3 + 1, help.length());
          }
        }
      }
    }// end of do
    while (location_3 >= 0);
    help="";
    int times[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    for (int a = 0; a<10; a++)
    {
      for (int i = 0; i<10; i++)
      {
        if ((Help[a] == Help[i])&&(Help[i]!=0))
        {
          times[a]++;
        }
      }
    } 
    int maxx = -10000; 
    int location_4 = 0;
    for (int a = 0; a<10; a++) 
    { if (times[a]>maxx) 
      { 
        maxx = times[a]; location_4 = a; 
      } 
    }
    // finding the max 
    */
    
    String result_s = "";

    // NO,VOL,cur,helping
    result_s += Num_device_0;  result_s += Num_device_1;  result_s += ";"; // device 
    result_s += results[1][0]; result_s += results[1][1]; result_s += results[1][2]; result_s += ",";// voltage value
    value = EEPROM.read(addr);
    if(value==1){
      result_s += "O"; result_s += "F"; result_s += "F"; result_s += "/";
    }
    else{
      result_s += results[0][0]; result_s += results[0][1]; result_s += results[0][2]; result_s += "/";
    }
    result_s += "00";  // Helping number = 00 (not use)
    /*
    if(Help[location_4]==device_noo){
      result_s+=0;
      result_s+=0;
    }
    else{
      if (Help[location_4]<10) 
         { result_s += 0; 
           result_s += Help[location_4];
         }// for format
      else {
          result_s += Help[location_4];
      }
    }
    */
    Serial.println(result_s); 
    Serial.print("Relay : ");
    
    Serial.print(value); 
    Serial.print(" / Payment : ");
    Serial.println(ddiiss_ppllaayy);

    RF.print(result_s);
    
    previous_result=result_s;
}

void loop(){ 
  sensing();
	communication();
  
  if(request){
    transmitting();
    count = 0;
    total_current = 0;
    total_voltage = 0;
    request = 0;
  }
  
	delay(1000);
}
  

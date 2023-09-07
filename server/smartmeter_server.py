### import module

import os
import csv
from openpyxl import load_workbook
from openpyxl import Workbook
import numpy as np
import datetime
from dateutil.parser import parse

import schedule
import serial

from datetime import datetime, timedelta
import pandas as pd
import pymysql

# Libraries for Serial communication and system time tracking
import time
import serial.tools.list_ports




#### define download (select) data from DB ####
# -----mySQL   # local computer DB server
dbName = "smartmeter"
dbHost = "127.0.0.1"
dbPort = 3306
dbUser = "root"
dbPW = "0000"

conn = pymysql.connect(host=dbHost,port=dbPort,user=dbUser,passwd=dbPW,db=dbName)

curs = conn.cursor()


####GSM part
start_time = time.time()
mid_time = 0
end_time = 600
begin_of_sms_flag = -1
middle_sms_separator = -1
ports = serial.tools.list_ports.comports()
gsmport = serial.Serial()
packet = ""
credit = 0
smartmeter_number = ""
phone_num = ""
portList = []
portNum = 11  # For GSM and Keypad Arduino MKR1000
portvar = ""

# List all available COM ports
for oneport in ports:
    portList.append(str(oneport))
    print(str(oneport))

# val = input("select Port: COM")

# Assign a COM port number to portvar variable
for x in range(0, len(portList)):
    if portList[x].startswith("COM" + str(portNum)):
        portvar = "COM" + str(portNum)
        print("\nSelected port is : " + portList[x])

gsmport.baudrate = 115200  # select GSM baudrate
gsmport.port = portvar  # Assign a static COM port number to GSM/Arduino module


# gsmport.open()

# Function to receive text message and slice it into credit and sender phone number
def sms_received():
    receive_flag = "+CMT"
    #    packet = ""
    if gsmport.in_waiting:
        packet = gsmport.readline()
        #   print(packet.decode('utf').rstrip('\n'))
        begin_of_sms_flag = packet.find(str.encode("*"))
        middle_sms_separator = packet.find(str.encode("#"))
        credit = packet[:begin_of_sms_flag].decode('UTF-8')
        now=datetime.now()
        if begin_of_sms_flag > 0 or middle_sms_separator > 0:
            # 'utf-8'gets rid of binary, spaces and carriage return character

            phone_num = packet[begin_of_sms_flag + 1:middle_sms_separator].rstrip().decode('UTF-8')
            #    print(str(begin_of_sms_flag) + str(middle_sms_separator))
            print(phone_num) # Variable to hold phone number of paid user
            print(credit[:]) # Variable to hold charged amount from User

            sm_query = "select SmartMeter_no from smartmeter.userinfo where Phone_no='" + str(phone_num) + "';"  # userinfo db에서 phone 넘버 매칭 스마티 미터 넘버
            curs.execute(sm_query)
            smartmeterno = curs.fetchone()
            d_query = "select credit_balance from smartmeter.credits where smartmeter_no='" + str(smartmeterno) + "';"  # credits db에서 crdeit_balance  불러오기
            curs.execute(d_query)
            creditba=curs.fetchone()
            credit_bal = int(int(creditba) +int(credit[:]/200))

            cd_query = "update smartmeter.credits set credit_balance='" + str(credit_bal) + "' where smartmeter_no='" + str(smartmeterno) + "';"
            curs.execute(cd_query)
            conn.commit()

            gsm_query = """insert into smartmeter.gsm (Datetime,Phone_no,amount,credit_charge,SmartMeter_no)values(%s,%s,%s,%s,%s)"""
            curs.execute(gsm_query, (now, phone_num, int(credit[:]), int(credit[:]/200), smartmeterno))
            conn.commit()

        if len(phone_num) < 12: # Assign Incoming prepayment digit to smartmeter number variable

            smartmeter_number = phone_num
            print(smartmeter_number)
            sm_query = "select SmartMeter_no from smartmeter.userinfo where Phone_no='" + str(smartmeter_number) + "';"  # userinfo db에서 phone 넘버 매칭 스마티 미터 넘버
            curs.execute(sm_query)
            smartmeterno = curs.fetchone()
            d_query = "select credit_balance from smartmeter.credits where smartmeter_no='" + str(smartmeterno) + "';"  # credits db에서 crdeit_balance  불러오기
            curs.execute(d_query)
            creditba = curs.fetchone()
            credit_bal = int(int(creditba) + int(credit[:] / 200))

            cd_query = "update smartmeter.credits set credit_balance='" + str(credit_bal) + "' where smartmeter_no='" + str(smartmeterno) + "';"
            curs.execute(cd_query)
            conn.commit()

            gsm_query = """insert into smartmeter.gsm (Datetime,Phone_no,amount,credit_charge,SmartMeter_no)values(%s,%s,%s,%s,%s)"""
            curs.execute(gsm_query, (now, phone_num, int(credit[:]), int(credit[:] / 200), smartmeterno))
            conn.commit()



## define base station serial port to Arduino #####

Base_PORT = 'COM59'
BaudRate = 9600

base_station= serial.Serial('COM'+Base_PORT,BaudRate)


print('serial ' + serial.__version__)



###### MAIN CODE ######

if __name__=='__main__':

    #inital credit get
    query = "select * from smartmeter.credits order by smartmeter_no desc;"
    val = curs.execute(query)
    print("smartmeter_no:",val)
    smartmeter_no = int(val)
    send = "##"
    for i in range(1,smartmeter_no+1):
        query = "select credit_balance from smartmeter.credits where smartmeter_no='"+str(i)+"';"  #credits db에서 crdeit_balance 전부 불러오기
        vals = curs.execute(query)
        creditb=curs.fetchone()
        # print("credit_balance:",creditb[0])
        # print('creditb[0]: ', creditb[0])
        send += str(creditb[0])
        send += ","
    print(send)
    base_station.write(str.encode(send))  # base station으로 신호 보냄

    # Always true loop function for checking incoming sms from GSM module through opening COM port
    while True:
        terminal_input = base_station.readline().decode('UTF-8')
        print('terminal: ', terminal_input)
        
        #PREPAYEMNT SYSTEM CODE
        if(terminal_input[0] == 'I'):
            #LOOK UP FROM THE DB
            card_id = terminal_input.split(',')
            query = "SELECT user_name, credit FROM smartmeter.prepayment where `user_ID` ='{}';".format(card_id[1]) #user_name, credit
            val = curs.execute(query)
            creditb=curs.fetchone()
            if creditb is None:
                comment = str("N-not registered")
                base_station.write(str.encode(comment))  # base station으로 신호 보냄
            else:
                if creditb[1] >0:  
                    comment = str("O;{},{}/".format(creditb[0]),format(creditb[1])) #  "O;Name,credit/" --> Open, send name and credit to arduino
                    base_station.write(str.encode(comment))
                else:
                    comment = str("C;{},{}/".format(creditb[0]),format(creditb[1])) #  "C;Name,credit/" --> Close, send name and credit to arduino
                    base_station.write(str.encode(comment))

        ##BASESTATION CODE
        if(terminal_input[0] == 'C'):
            print("cycle done")
            query = "select * from smartmeter.credits order by smartmeter_no desc;"
            val = curs.execute(query)
            # print("smartmeter_no:", val)
            print("total smartmeter no:", val)
            smartmeter_no = int(val)
            send = "##"
            for i in range(1, smartmeter_no+1):
                query = "select * from smartmeter.credits where smartmeter_no='" + str(i) + "';"
                vals = curs.execute(query)
                creditb = curs.fetchone()
                print("credit_balance:", creditb[3])
                send += str(int(creditb[3]))
                send += ","
            print(send)

            base_station.write(str.encode(send))  # base station으로 신호 보냄

        elif(terminal_input[0] == 'D'): # smartmeter to db 에 로그 저장 + credit db 업데이트
            print("get data")

            d = terminal_input.split(',') # [D 1 2031 220 003]
            print(d)
            channel_no1= d[0]
            home_no = int(d[1])       # home_no
            timegap = float(int(d[2])/1000)  # unit : s
            voltage = float(d[3])  # unit : V
            current = float(int(d[4])/100)  # unit : A
            power=voltage*current
            energy=power*timegap/3600
            now=datetime.now()
            # smartmeter to db에 형식에 맞게 query 쏴주기
            s2d_query = """insert into smartmeter.smartmeter2db(Datetime,Channel_no,SmartMeter_no,voltage,current,Power_kwh)values(%s,%s,%s,%s,%s,%s)"""
            curs.execute(s2d_query, (now,channel_no1,home_no,voltage,current,energy))
            conn.commit()

            # credit db에서 현재 값 불러오고, 계산된 power에 맞게 뺄셈해서 업데이트
            # d_query = "select credit_balance from smartmeter.credits where channel_no='" + Base_PORT + "' and smartmeter_no='" + str(home_no) + "';"  # credits db에서 crdeit_balance 전부 불러오기
            # # print('d_query: ',d_query)
            # curs.execute(d_query)
            # credit=curs.fetchone()
            # credit_bal=int(int(credit[0])- energy)
            # c_query = "update smartmeter.credits set credit_balance='"+str(credit_bal)+"' where smartmeter_no='"+str(home_no)+"';"  ## updata calculated credit by using power
            # curs.execute(c_query)
            # conn.commit()

        # try:
        #     if not gsmport.isOpen():
        #         gsmport.open()
        #     sms_received()
        #
        # except Exception:
        #     print("No port detected with specified COM")
        #
        # if start_time - mid_time >= end_time:
        #     gsmport.write(str.encode('AT+CMGD=1,4\r\n'))
        #     mid_time = start_time
        # start_time = time.time()

        # time.sleep(60)
conn.close()





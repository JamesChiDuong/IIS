#!/usr/bin/env python3
import serial
import sys
import time
import curses

CHECK = True
#Inform you port example:/dev/pts/2
if len(sys.argv) < 2:
  print("Usage: {0} [DEVICE]".format(sys.argv[0]))
  sys.exit(-1)


dev = serial.Serial(sys.argv[1], 115200)      #Open serial port

message = str(sys.argv[2])      #Format message
print("Send Data: ",message)
message = message + '\n'
dev.write(message.encode())                 #Send message throuhg Serial Port
dataRaw = dev.readline()                    #Waiting to read data
data = dataRaw.decode('utf8')               #Decode to string
print("Received Data: ",data)               #print
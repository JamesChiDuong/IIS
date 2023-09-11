#!/usr/bin/env python3
import serial
import sys
import time
import curses
#Inform you port example:/dev/pts/2

operand_str = ""
operand = 0
output_number1 = 0
output_number2 = 0
output_result = 0
numOfByte = 0
BITWISE_12BIT = 0xffffffffffff

def twos_comp(val, bits):
    """compute the 2's complement of int value val"""
    if (val & (1 << (bits - 1))) != 0: # if sign bit is set e.g., 8bit: 128-255
        val = val - (1 << bits)        # compute negative value
    return val

def get_hex(value):
  convert_string = int(value)
  convert_hex = hex(convert_string)
  return convert_hex

def get_byte(value,lengOfByte):
  print("The length of byte: ",int(lengOfByte))
  return value.to_bytes(int(lengOfByte),'big')

if len(sys.argv) < 2:
  print("Usage: {0} [DEVICE]".format(sys.argv[0]))
  sys.exit(-1)


dev = serial.Serial(sys.argv[1], 115200)  #Open serial port
if((sys.argv[2].isdigit()== False or sys.argv[3].isdigit()== False) or (int(sys.argv[2]) and int(sys.argv[3]) > 255)):
  print("Error: Please input number or input the data from 0-255")
  exit()

if(sys.argv[4] == "add"):
  input_operand = 0x1
elif(sys.argv[4] == "sub"):
  input_operand = 0x2
elif(sys.argv[4] == "mul"):
  input_operand = 0x3
elif(sys.argv[4] == "div"):
  input_operand = 0x4
else:
  print("Error")
  exit()


input_data = (int(sys.argv[2]) << 32) | ((int(sys.argv[3]) << 24) | ((int(input_operand) << 16)) | (0x0C<<8) | 0x0A)
if(int(sys.argv[2]) <100):
  numOfByte = (((input_data.bit_length()+1))/8)+1
else:
  numOfByte = (((input_data.bit_length()+1))/8)
#input_data = (int(sys.argv[2]) << 32) | ((int(sys.argv[3]) << 24) | ((int(input_operand) << 16) | 0x0A))
message = get_byte(input_data,numOfByte)

print("Send Data: ",hex(input_data))
dev.write(message)                #Waiting to read data


dataRaw = dev.readline()

integer_big_endian = int.from_bytes(dataRaw, 'big')
hex_data = hex(integer_big_endian)

data_filter = integer_big_endian & BITWISE_12BIT

numberOfbyte = ((integer_big_endian.bit_length()+2)/8)


if(numberOfbyte == 8):
  output_number1 = data_filter >> 40########& (BITWISE_8BIT)


  output_number2 = (data_filter >>32) & 0xFF


  operand = (data_filter >> 24) & 0xFF

  output_result = (data_filter >> 8) & 0xFFFF

elif(numberOfbyte == 7):
  output_number1 = (data_filter >> 32) & 0xFF########& (BITWISE_8BIT)


  output_number2 = (data_filter >>24) & 0xFF


  operand = (data_filter >> 16) & 0xFF

  output_result = (data_filter)  & 0xFF

if(operand == 0x1):
   operand_str = "add"
elif(operand == 0x2):
   operand_str = "sub"
   if(int(sys.argv[2]) < int(sys.argv[3])):
     output_result = twos_comp(output_result,(output_result.bit_length()))
elif(operand == 0x3):
   operand_str = "mul" 
elif(operand == 0x4):
   operand_str = "div"

print("Number of Bytes:", numberOfbyte)

print("Received Data: ",hex_data)             #print

print("Number1: ", hex(output_number1), output_number1)
print("Number2: ", hex(output_number2), output_number2)
print("Operand: ", hex(operand), operand_str)
print("Result: ", hex(output_result), output_result)

#data = dataRaw.decode("utf8")
#hex = hex(data)
#hex_int = int(data,16)
#new_int = data + 0x200
#data = dataRaw.decode('utf8').strip()            #Decode to string
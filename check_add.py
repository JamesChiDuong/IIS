import sys
import argparse
import os.path
from parse import parse
#**************************User-Interface****************************#
global_parser = argparse.ArgumentParser(prog="calc")
subparsers = global_parser.add_subparsers(dest = "command",
    title="subcommands"
)
arg_writefile_template = {
    "dest": "filename",
    "metavar": "FILE",
    "help": "Implement with file including write and write a file",
}

writefile_parser = subparsers.add_parser("writefile", help="Syntax to writefile: python3 check_add.py writefile myfile1.txt 'content' ")
writefile_parser.add_argument(**arg_writefile_template)
writefile_parser.add_argument('text', help='The text to write to the file.')

args = global_parser.parse_args()

#**************************Main_Function*****************************#
def InverseTwoComplement(numberInput,bits):
        if((numberInput & (1 << (bits - 1)))!= 0):
            numberInput = numberInput - (1 << bits)
        return numberInput
def CheckSumOfInterger(a,b,c,sum,e):
     global number1
     global number2
     global Sum
     global Cin
     global Cout
     global overloadVariable
     number1 = InverseTwoComplement(a,8)
     number2 = InverseTwoComplement(b,8)
     Sum = InverseTwoComplement(sum,8)
     Cin = c
     Cout = e
     if(((number1 + number2 + int(c)) > 127) or ((number1 + number2 + int(c)) < -127)):
        overloadVariable = 1
     else:
        overloadVariable = 0
     if((number1 + number2 + int(c)) == Sum):
        return True
     else:
        return False
if args.command == "writefile":
    with open(args.filename, 'w') as file:
        file.write(args.text)
else:
    for line in sys.stdin:
        
        result = parse("{:b} {:b} {:d} {:b} {:d}", line.strip())
        if result is not None:
            a,b,c,d,e = result
            if(CheckSumOfInterger(a,b,c,d,e) == True):
                print("\nOUTPUT_Inform:\t\t\t---->The test is CORRECT!!!<----\n")
                print("\nDecimal Format\n\n\t-->First Number: %d\n\t-->Second Number: %d\n\t-->C_input: %s\n\t-->Sum: %d\n\t-->C_output: %s\n" %(number1,number2,Cin,Sum,Cout))
            else:
                print("\nOUTPUT_Inform:---->The result is INCORRECT!!!!<----\n")
            if(overloadVariable == 1):
                print("\nOUTPUT_Inform:---->OVERLOAD Variable!!!!<----\n")
                print("\nDecimal Format\n\n\t-->First Number: %d\n\t-->Second Number: %d\n\t-->C_input: %s\n\t-->Sum: %d\n\t-->C_output: %s\n" %(number1,number2,Cin,Sum,Cout))
        else:
        # Handle the error case here
            print("Error!!!!")
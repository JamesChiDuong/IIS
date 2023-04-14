import argparse
import os.path
import sys
#**************************User-Interface****************************#
def add(a, b):
    return a + b

def openfile(parser, arg):
    if not os.path.exists(arg):
        parser.error("The file %s does not exist!" % arg)
    else:
        return open(arg, 'r')  # return an open file handle

global_parser = argparse.ArgumentParser(prog="calc")
subparsers = global_parser.add_subparsers(dest = "command",
    title="subcommands", help="arithmetic operations"
)
arg_template = {
    "dest": "operands",
    "type": int,
    "nargs": 2,
    "metavar": "OPERAND",
    "help": "a numeric value",
}
arg_readfile_template = {
    "dest": "filename",
    "type": lambda x: openfile(readfile_parser, x),
    "metavar": "FILE",
    "help": "Implement with file including write and read a file",
}
arg_writefile_template = {
    "dest": "filename",
    "metavar": "FILE",
    "help": "Implement with file including write and write a file",
}
add_parser = subparsers.add_parser("gen", help="Syntax to add the number: python3 gen_input.py gen 2 4")
add_parser.add_argument(**arg_template)
add_parser.set_defaults(func=add)

readfile_parser = subparsers.add_parser("readfile", help="Syntax to readfile: python3 gen_input.py readfile myfile1.txt")
readfile_parser.add_argument(**arg_readfile_template)

writefile_parser = subparsers.add_parser("writefile", help="Syntax to writefile: python3 gen_input.py writefile myfile1.txt 'content' ")
writefile_parser.add_argument(**arg_writefile_template)
writefile_parser.add_argument('text', help='The text to write to the file.')

args = global_parser.parse_args()

if args.command == "gen":
    sys.stdout.write(f"{args.operands[0]} {args.operands[1]}\n")
if args.command == "readfile":
    for line in args.filename:
        print(line.strip())
if args.command == "writefile":
    with open(args.filename, 'w') as file:
        file.write(args.text)

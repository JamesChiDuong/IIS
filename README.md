# PLATFORM FOR THE FPGA PROJECT
Beside coding with the verilog code, we need to make the simulation program and test program in a big project.
In this source, we will divide for 3 part: The part for rtl code, the part for simulation code, the part for test code.
Each part will comunicate via UART protocol and open the Pseudo Teminal

- rtl folder: To store the rtl file(verilog file,hdvl file,...). In this folder, it will have a three UART modules, one for transmit, one for receive and one for baud rate genertator. The rest of module is the Full Adder module which will be tested when receiving the data from python script and after calculating it send back the data to the python script via UART protocol.
- bench folder: Include cpp folder and verilog folder. With verilog folder will store the main verilog code which need to test bench. Cpp folder to store the Cpp file which use the simulation by system verilog. Run the makefile to compile and run program.
- Test Folder: Store the python file, which is used to test the verilog program

**The command to combine and Run the code**
- Compile and Run the files in the rtl file: make rtl
- Compile and Run the files in the bench folder: make bench
- Compile and Run the files in the cpp folder: make test

**The Process to Test**
- After run the make test, the program will be opened the port with the name /dev/pts/
- We need to open the new terminal and run the python script with the command: python3 Test_Data_Receiver.py
Reference: https://github.com/ZipCPU/wbuart32

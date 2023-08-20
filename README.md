# SECTION 1: Project's Structure and How to Use the Makefile

## Project's Structure
The project is a GNU Makefile project.
The modules are written in Verilog and parameterized with built-in Verilog mechanisms, SageMath, and Python scripts.
The makefile at the top includes targets for the top modules:
    TranAndRecei  Data Receiver.
The makefile project automates the source, simulation, synthesis, implementation, generate the bitstream, and program the FPGA(for Xilinx devices) 

The makefile structure enables the automated building process
for the various specified parameter sets, module configurations, and FPGA models. 

The purpose of the individual folders is as follows:

| Folder          | Description                                                                |
| --------------- | -------------------------------------------------------------------------- |
| `build/`        | Build folder including generated sources, results, and simulation file.                                                                      |
| `host/`         | Python test code. It includes 2 files. The Test_Data_Receiver.py uses to test Data_Receiver modules and the Test_TranAndRecei.py is used to test TranAndRecei modules                                                                    |
| `modules/`      | User HDL code. This includes these top modules Verilog file and modules/FPGA folder.                                                               |
| `platform/`     | simulation CPP code is used to simulate these top modules, including `platform/cpp` and `platform/rtl`. The folder cpp has a cpp file to simulate via Verilator. The folder rtl has these modules for UART communication                     | 
| `Makefile`      | Top Makefile.                                          |
| `config.mk`     | To make sure we need to move the previous target when running the new target|
| `FPGA.mk`       | To run for TARGET = ArtixA7 for synthesis, implement, generate bit-stream, and program for FPGA                                            |
| `simulation.mk` | To run for the TARGET = sim for simulation             |
| `target.mk`     | To set the target for the top make file                    |

## How to use the Makefile

In this source code, I will use 2 top modules to test these cases.
- The Data_Receiver.v is the top module with testing to transfer data from the keyboard and receive back the data via UART protocol. We can test the simulation with many rounds

- The TranAndRecei.v and alu.v is the two modules for the purpose that we send two data from the Python test file via UART protocol. The TranAnRecei.v is a top module. The alu module is the additional submodule. The two data which is sent by Python Test files are the input for the alu modules, after calculating by full adder modules, these data include two data, the sum of two data will be sent back to the Python test file via UART protocol. In the simulation with Verilator, we can test the simulation in many rounds but In the FPGA, we can only test one round. Because in the Verilog file, I only implement the one-round code.

### Target 'sim':

  ```bash
  make TARGET=sim
  ```
  When we run the command:

   The program will generate the `build/simulation/cpp`, `build/simulation/rtl`, `/build/simulation/verilog` and will run `.mk file` of each folder. After running, the terminal will compile and run the code. It will open the pseudo-terminal and wait for the test file from `host/` folder.

   `Example 1:`
  
  | TOP MODULE FILE          |      TEST PYTHON FILE                                      |
  | ---------------          |     --------------------------------------------------------------------------              |
  |`./Data_Receiver`         | `python3 Test_Data_Receiver.py /dev/pts/4 hello`           |
  | Slave device: /dev/pts/4 |  Send Data:  hello                                         |
  | Received 6 bytes: hello                                                              
  | Successfully read 6 characters: hello                                                 | 
  | Sent 6 bytes: hello      |  Received Data:  hello                                     |
  |We can't stop the program expect we interrupt the program |                            |

  `Example 2:`
  We will use Python scripts decides which operands will be choose. We will implement with `addition, subtraction, and multiplicaton` to test with design.
  - The syntax `additional`: python3 Test_TranAndRecei.py /dev/pts/4 110 101 add
  - The syntax `additional`: python3 Test_TranAndRecei.py /dev/pts/4 110 101 sub
  - The syntax `additional`: python3 Test_TranAndRecei.py /dev/pts/4 110 101 mul
  - The syntax `additional`: python3 Test_TranAndRecei.py /dev/pts/4 110 101 div
  | TOP MODULE FILE                 |      TEST PYTHON FILE                                      |
  | ---------------                 |     --------------------------------------------------------------------------              |
  |`./TranAndRecei`                 | `python3 Test_TranAndRecei.py /dev/pts/4 110 101 add`      |
  | Slave device: /dev/pts/4        | Send Data:  112-002-add                                    |
  | Received 12 bytes: 112-002-add  |
  |Successfully read 50 characters:   NUMBER1:112 NUMBER2:002 OPERAND:add RESULT:00114
  |Sent 50 bytes:   NUMBER1:112 NUMBER2:002 OPERAND:add RESULT:00114
PASS!                               |  Received Data:  NUMBER1:112 NUMBER2:002 OPERAND:add RESULT:00114    |
  | We can't stop the program expect we interrupt the program                             |

#### NOTE:
If you want to run the simulation of Data_Receiver modules. Go into the folder `platform/cpp/cpp` and remove the `#` character at test target.
### Target 'ArtixA7':

  ```bash
  make TARGET=ArtixA7
  ```
  When we run the command:
  
  The program will generate the `build/Artycs324g/Top_moduleName/src` and `build/results`. The `build/Artycs324g/Top_moduleName/src` has all the Verilog files on your project. The `build/results` has the log file which stores the command line on the bash.

   `Example 1:`
  | TOP MODULE FILE          |      TEST PYTHON FILE                                      |
  | ---------------          |     --------------------------------------------------------------------------              |
  |`./Data_Receiver`         | `python3 Test_Data_Receiver.py /dev/ttyUSB1`               |
  |                          | Send Data:  Hello from Python file                         |
  |                          | Received Data:  Hello from Python file                     |

   `Example 2:`
  | TOP MODULE FILE          |      TEST PYTHON FILE                                      |
  | ---------------          |     --------------------------------------------------------------------------              |
  |`./TranAndRecei`          | `python3 Test_TranAndRecei.py /dev/ttyUSB1 110 101 add`        |
  |                          | Send Data:  110-101-add                                        |
  |                          | Received Data:  NUMBER1:110 NUMBER2:101 OPERAND:add RESULT:00211     |
  
  ## NOTE:
   - We need to use `make clean` before running the new target
   - To list the USB port of your device, open the terminal: `sudo ls /dev/ttyUSB*`


# SECTION 2: What needs to be changed to adapt the code for another design
  
  ## With TARGET=sim

   - The `modules/` folder to change the top modules' Verilog files

   - The `platform/cpp` to change the simulation top modules file
   
   - The `modules/verilog.mk`,`platform/cpp/cpp.mk` to change the name of the modules at the MODULES and MODULES2 variables in this file.

  ## With TARGET=ArtixA7

   - Make sure you have already installed follow `modules/FPGA/tools.mk`

   - Plug your FPGA board

   - The `FPGA.mk` to change the top modules at TOPMODULES variable

   - The `modules/FPGA/parameter.mk` to change the package following your FPGA. 
   Example: my FPGA board is Artix-7 with package csg324
   
   - The `modules/FPGA/Xilinx/models` to change the Xilinx model. 
   Example: my FPGA board is xc7a100tcsg324-1 model

   - The `modules/FPGA/Xilinx/pin_artix7_100t.xdc` to configure the pin planner for your FPGA

   - Another file in `modules/FPGA/Xilinx/timing__` to create the clock for FPGA, we don't need to change

   - The `modules/modules.mk/` to change the top modules. 
   Example: If I want to program FPGA with Data_Receiver is a top module. I only change the name of the top module at TOPMODULE and TOPMODULE_CHECK variable and also delete the fullAdder.v at MODULESTOP_SRC variable because, in the Data_Receiver, I don't use the alu module.



  

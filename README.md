# Project's Structure
The project is a GNU Makefile project.
The modules are written in Verilog and parameterized with built-in Verilog mechanisms, SageMath, and Python scripts.
The makefile at the top includes targets for the top modules:
    TranAndRecei  Data Receiver.
The makefile project automates the source, simulation, the synthesis, implement, generate bitstream and program the FPGA(for Xilinx devices) 

The makefile structure enables the automated building process
for the various specified parameters sets,module configarations, and FPGA models. 

The purpose of the individual folders are as follow:

| Folder          | Description                                                                |
| --------------- | -------------------------------------------------------------------------- |
| `build/`        | Build folder including generated sources, results and simulation file.                                                                      |
| `host/`         | Python test code. It includes 2 files. The Test_Data_Receiver.py uses to     test Data_Receiver modles and the Test_TranAndRecei is used to test TranAndRecei modules                                                                    |
| `modules/`      | User HDL code. This includes these top modules Verilog file and modules/FPGA folder.                                                       |
| `platform/`     | simulation CPP code is used to simulation these top modules, include platform/cpp and platform/rtl. The folder cpp has cpp file to simulation via Verilator. The folder rtl has these modules for UART comunication                         | 
| `Makefile`      | Top Makefile.                                          |

### Example TEST
In this source code, I will use 2 top module to test these case.
- The Data_Receiver.v is the top modules with testing transfer data from keyboard and receive back the data via UART protocol. We can test simulation with many round

- The TranAndRecei.v and fullAdder.v is the two modules for the purpose that we send two data from python test file via UART protocol. The TranAnRecei.v is a top module. The fullAdder is the additional submodule.The two data which is sent by Python Test files is the input for the full adder modules, after calculated by full adder modules, these data includes two data, sum of two data will be sent back to the python test file via UART protocol.We can test simulation one round

### Target 'sim':

  ```bash
  make TARGET=sim
  ```
   Before run the command:

   - The `modules/` folder to change the top modules Verilog files

   - The `platform/cpp` to change the simulation top modules file
   
   - The `modules/verilog.mk`,`platform/cpp/cpp.mk` to change the name of the modules at the MODULES and MODULES2 variable in this file.


   When we run command:

   The program will generate the `build/simulation/cpp`, `build/simulation/rtl`, `/build/simulation/verilog` and will run `.mk file` of each folder. After running, the terminal will compile and run the code. It will open the pseudo-terminal and waiting the test file from `host/` folder.

   `Example 1:`
  | TOP MODULE FILE          |      TEST PYTHON FILE                                      |
  | ---------------          |     --------------------------------------------------------------------------              |
  |`./Data_Receiver`         | `python3 Test_Data_Receiver.py /dev/pts/4`                 |
  | Slave device: /dev/pts/4 | Send Data:  Hello from Python file                         |
  | Received 23 bytes: Hello from Python file EOF 
  | Successfully read 23 characters: Hello from Python file| 
  | Sent 23 bytes: Hello from Python file |Received Data:  Hello from Python file         |

  `Example 2:`
  | TOP MODULE FILE          |      TEST PYTHON FILE                                      |
  | ---------------          |     --------------------------------------------------------------------------              |
  |`./TranAndRecei`          | `python3 Test_TranAndRecei.py /dev/pts/4`                  |
  | Slave device: /dev/pts/4 | Send Data:  110 101 EOF                                    |
  | Received 9 bytes:        |
  |110 101 EOF               |
  |Successfully read 40 characters:   NUMBER1:110 NUMBER2:101 SUM:211 COUT:0
  |Sent 40 bytes:   NUMBER1:110 NUMBER2:101 SUM:211 COUT:0
PASS!                        |  Received Data:  NUMBER1:110 NUMBER2:101 SUM:211 COUT:0    |


### Target 'ArtixA7':

  ```bash
  make TARGET=ArtixA7
  ```

  Before run the command:
   - Make sure you have already installed follow `modules/FPGA/tools.mk`

   - Plug your FPGA board

   - The `modules/FPGA/parameter.mk` to change the package following your FPGA. Example: my FPGA board is Artix-7 with package csg324
   
   - The `modules/FPGA/Xilinx/models` to change the Xilinx model.Example: my FPGA boatd is xc7a100tcsg324-1 model

   - The `modules/FPGA/Xilinx/pin_artix7_100t.xdc` to configure the pin planner for your FPGA

   - Another files in `modules/FPGA/Xilinx/timing__` to create the clock for FPGA, we don't need to change

   - The `modules/modules.mk/` to change the top modules. Example: If I want to program FPGA with Data_Receiver is a top modules. I only change the name of top module at TOPMODULE and TOPMODULE_CHECK variable and also delete the fullAdder.v at MODULESTOP_SRC variable because in the Data_Receiver, I don't use the fullAdder module.


  
  When we run command:
  
  The program will generate the `build/Artycs324g/Top_moduleName/src` and `build/results`. The `build/Artycs324g/Top_moduleName/src` has all the Verilog files on your project. The `build/results` has the log file which store the command line on the bash.

   `Example 1:`
  | TOP MODULE FILE          |      TEST PYTHON FILE                                      |
  | ---------------          |     --------------------------------------------------------------------------              |
  |`./Data_Receiver`         | `python3 Test_Data_Receiver.py /dev/ttyUSB1`               |
  |                          | Send Data:  Hello from Python file                         |
  |                          | Received Data:  Hello from Python file                     |

   `Example 2:`
  | TOP MODULE FILE          |      TEST PYTHON FILE                                      |
  | ---------------          |     --------------------------------------------------------------------------              |
  |`./TranAndRecei`          | `python3 Test_TranAndRecei.py /dev/ttyUSB1`                |
  |                          | Send Data:  110 101                                        |
  |                          | Received Data:  NUMBER1:110 NUMBER2:101 SUM:211 COUT:0     |

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
| `modules/`      | User HDL code. This includes these top modules Verilog file and modules/FPGA folder. The Data_Receiver.v is the top modules with testing transfer data from keyboard and receive back the data via UART protocol. The TranAndRecei.v and fullAdder.v is the 2 modules for the purpose that we send 2 data from python test file via UART protocol, and then 2 data is the input for the full adder modules, after calculated by full adder modules, the 2 data, sum of 2 data will be sent back to the python test file via UART protocol. The modules/FPGA has some files to configure parameter to synthesis, implement, generate the bit stream and program for FPGA                                                       |
| `platform/`     | simulation CPP code is used to simulation these top modules, include platform/cpp and platform/rtl. The folder cpp has cpp file to simulation via Verilator. The folder rtl has these modules for UART comunication                         | 
| `Makefile`      | Top Makefile.                                          |

### Target 'sim':

  ```bash
  make TARGET=sim
  ```
   Before run the command:

   _-The `/modules` folder to change the top modules Verilog files_

   _-The `platform/cpp` to change the simulation top modules file_ 
   
   _-The `modules/verilog.mk`,`platform/cpp/cpp.mk` to change the name of the modules at the MODULES and MODULES2 variable in this file._


   When we run command, the program will generate the `build/simulation/cpp`, `build/simulation/rtl`, `build/simulation/verilog` and will run `.mk file` of each folder. After running, the terminal will compile and run the code. It will open the pseudo-terminal and waiting the test file from `/host` folder.

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
   _- Plug the FPGA board_
   _We will go into the `/modules` folder to change the top modules Verilog files, go into `platform/cpp` to change the simulation top modules file and also go into the `modules/verilog.mk`,`platform/cpp/cpp.mk` to change the name of the modules at the MODULES and MODULES2 in this file._
  
  When we run command, the program will generate the `build/Artycs324g/Top_moduleName/src` and `build/results`. The `build/Artycs324g/Top_moduleName/src` has all the Verilog files on your project. The `build/results` has the log file which store the command line on the bash. 

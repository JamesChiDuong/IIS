# Project's Structure
The project is a GNU Makefile project.
The modules are written in Verilog and parameterized with built-in Verilog mechanisms, SageMath, and Python scripts.
The makefile at the top includes targets for the top modules:
    TranAndRecei  Data Receiver.
The makefile project automates the source, simulation, synthesis, and implementation, generates the bitstream, and programs the FPGA(for Xilinx devices) 

The makefile structure enables the automated building process
for the various specified parameter sets, module configurations, and FPGA models. 

The purpose of the individual folders is as follows:

| Folder          | Description                                                                |
| --------------- | -------------------------------------------------------------------------- |
| `build/`        | Build folder including generated sources, results, and simulation file.                                                                      |
| `host/`         | Python test code. It includes 2 files. The Test_Data_Receiver.py uses to test Data_Receiver modules and the Test_TranAndRecei.py is used to test TranAndRecei modules                                                                    |
| `modules/`      | User HDL code. This includes these top modules Verilog file and modules/FPGA folder.                                                               |
| `platform/`     | simulation CPP code is used to simulate these top modules, including `platform/cpp` and `platform/rtl`. The folder cpp has a cpp file to simulate via Verilator. The folder rtl has these modules for UART communication                         | 
| `Makefile`      | Top Makefile.                                          |

### Example TEST
In this source code, I will use 2 top modules to test these cases.
- The Data_Receiver.v is the top module with testing transfer data from the keyboard and receiving back the data via UART protocol. We can test simulation with many rounds

- The TranAndRecei.v and fullAdder.v is the two modules for the purpose that we send two data from the Python test file via UART protocol. The TranAnRecei.v is a top module. The full adder is the additional submodule. The two data which is sent by Python Test files are the input for the full adder modules, after calculating by full adder modules, these data include two data, the sum of two data will be sent back to the Python test file via UART protocol. In the simulation with Verilator, we can test the simulation in many rounds but In the FPGA, we can only test one round. Because in the Verilog file, I only implement the one-round code.

### Target 'sim':

  ```bash
  make TARGET=sim
  ```
   Before running the command:

   - The `modules/` folder to change the top modules Verilog files

   - The `platform/cpp` to change the simulation top modules file
   
   - The `modules/verilog.mk`,`platform/cpp/cpp.mk` to change the name of the modules at the MODULES and MODULES2 variables in this file.


   When we run the command:

   The program will generate the `build/simulation/cpp`, `build/simulation/rtl`, `/build/simulation/verilog` and will run `.mk file` of each folder. After running, the terminal will compile and run the code. It will open the pseudo-terminal and waiting for the test file from `host/` folder.

   `Example 1:`
  | TOP MODULE FILE          |      TEST PYTHON FILE                                      |
  | ---------------          |     --------------------------------------------------------------------------              |
  |`./Data_Receiver`         | `python3 Test_Data_Receiver.py /dev/pts/4`                 |
  | Slave device: /dev/pts/4 | Input data Test: Hello from Python file                    |
  |                          | Send Data:  Hello from Python file                         |
  | Received 23 bytes: Hello from Python file EOF 
  | Successfully read 23 characters: Hello from Python file| 
  | Sent 23 bytes: Hello from Python file |Received Data:  Hello from Python file         |
  |                          | Input data Test: out                                       |
  | End of round             |                                                            |

  `Example 2:`
  | TOP MODULE FILE          |      TEST PYTHON FILE                                      |
  | ---------------          |     --------------------------------------------------------------------------              |
  |`./TranAndRecei`          | `python3 Test_TranAndRecei.py /dev/pts/4 110 101`          |
  | Slave device: /dev/pts/4 | Send Data:  110 101 EOF                                    |
  | Received 9 bytes:        |
  |110 101 EOF               |
  |Successfully read 40 characters:   NUMBER1:110 NUMBER2:101 SUM:211 COUT:0
  |Sent 40 bytes:   NUMBER1:110 NUMBER2:101 SUM:211 COUT:0
PASS!                        |  Received Data:  NUMBER1:110 NUMBER2:101 SUM:211 COUT:0    |
  | We can't stop the program expect we interrupt the program                             |


### Target 'ArtixA7':

  ```bash
  make TARGET=ArtixA7
  ```

  Before running the command:
   - Make sure you have already installed follow `modules/FPGA/tools.mk`

   - Plug your FPGA board

   - The `modules/FPGA/parameter.mk` to change the package following your FPGA. Example: my FPGA board is Artix-7 with package csg324
   
   - The `modules/FPGA/Xilinx/models` to change the Xilinx model.Example: my FPGA boatd is xc7a100tcsg324-1 model

   - The `modules/FPGA/Xilinx/pin_artix7_100t.xdc` to configure the pin planner for your FPGA

   - Another file in `modules/FPGA/Xilinx/timing__` to create the clock for FPGA, we don't need to change

   - The `modules/modules.mk/` to change the top modules. Example: If I want to program FPGA with Data_Receiver is a top module. I only change the name of the top module at TOPMODULE and TOPMODULE_CHECK variable and also delete the fullAdder.v at MODULESTOP_SRC variable because, in the Data_Receiver, I don't use the fullAdder module.


  
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
  |`./TranAndRecei`          | `python3 Test_TranAndRecei.py /dev/ttyUSB1 110 101`        |
  |                          | Send Data:  110 101                                        |
  |                          | Received Data:  NUMBER1:110 NUMBER2:101 SUM:211 COUT:0     |

  ### NOTE:
  To list the USB port of your device, open the terminal: `sudo ls /dev/ttyUSB*`

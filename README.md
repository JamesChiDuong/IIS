# Project's Structure
The project is a GNU Makefile project.
The modules are written in Verilog and parameterized with built-in Verilog mechanisms, SageMath, and Python scripts.
The makefile at the top includes targets for the top modules:
    TranAndRecei Data_Transmitter Data Receiver.
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
   If we want to change the top module. We will go into the `/modules` folder to change the top modules Verilog files, go into `platform/cpp` to change the simulation top modules file and also go into the `modules/verilog.mk` and `platform/cpp/cpp.mk` to change the name of the modules. 
   
   When we run command, the program will generate the `build/simulation/cpp`, `build/simulation/rtl`, `build/simulation/verilog` and will run each `.mk file` of each folder. After running, the terminal will compile and run the code. It will open the pseudo-terminal and waitting the test file from `/host folder` test.

   `Example:`
   | FILE                     | RESPONSE                           |
   |`./Data_Receiver`         | Slave device: /dev/pts/4 simulation                                                      |
   | `python3 Test_Data_Receiver.py /dev/pts/4`| python3 Test_Data_Receiver.py /dev/pts/                                                            |




### Target 'ArtixA7':

  ```bash
  make TARGET=ArtixA7
  ```

  To synthesis, implement, generate a bitstream and program of the FPGA project.

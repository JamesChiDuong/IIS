Items within this directory include:
- uartsim defines a C++ class that can be used for simulating a UART within Verilator and define the Pseudo Terminal. This class can be used both to generate valid UART signaling, to determine
if you configuration can receive it properly, as well as to decode valid UART signaling to determind if your configuration is properly setting the UART signaling wire
- Demonstration projects using these: --Data_Receiver excercises and tests the Data_Receiver.v, --Data_Transmitter excercises and tests the Data_Transmitter.v, -- Top excercises and test the Top.v
This is also creates a .VCD file which can be viewed via GTKwave

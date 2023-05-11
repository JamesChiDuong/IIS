This directory contains three basic configurations for testing UART with Pseudo Terminal and proving that its works:
- Data_Transmitter.v: Display the familiar "Hello world" message. Test the transmit via UART port
- Data_Receiver.v: Echoes any character recived directly back to the transmit port. It will receive the Data from Python script and send it back.
- Top.v: We add the top-level module that receives inputs from the host PC ( Python script or UART tool) via Serial communication, forward the input data to the FullAdder module and sends the result back
via serial communication to the host for checking

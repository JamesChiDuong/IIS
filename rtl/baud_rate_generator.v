/***Set up for 9600 baudrate

For 9600 baud with 100MHz FPGA clock: 
9600 * 16 = 153,600
100 * 10^6 / 153,600 = ~651      (counter limit M)
log2(651) = 10                   (counter bits N) 

 For 19,200 baud rate with a 100MHz FPGA clock signal
 19,200 * 16 = 307,200
 100 * 10^6 / 307,200 = ~326      (counter limit M)
 log2(326) = 9                    (counter bits N)

 For 115,200 baud with 100MHz FPGA clock
 115,200 * 16 = 1,843,200
 100 * 10^6 / 1,843,200 = ~52     (counter limit M)
 log2(52) = 6                     (counter bits N)
*/
module baud_rate_generator
    #(       
        parameter   N = 6,     // number of counter bits
                    M = 53     // counter limit value
    )
    (
        input clk,       
        input reset,            // reset
        output tick             // sample tick
    );
    
    // Counter Register
    reg [N-1:0] counter;        // counter value
    wire [N-1:0] next;          // next counter value
    
    // Register Logic
    always @(posedge clk, posedge reset)
        if(reset)
            counter <= 0;
        else
            counter <= next;
            
    // Next Counter Value Logic
    assign next = (counter == (M-1)) ? 0 : counter + 1;
    
    // Output Logic
    assign tick = (counter == (M-1)) ? 1'b1 : 1'b0;
       
endmodule
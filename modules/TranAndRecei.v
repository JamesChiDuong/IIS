
/* verilator lint_off UNUSED */
module TranAndRecei (
    input clk,
    input i_uart_rx,
    input reset,
    /* verilator lint_off UNDRIVEN */
    output o_uart_tx
);
localparam  DBITS = 8,                                  // 8 bit Data
            DATA_LENGTH = 8,                           // String length of the Data buffer    
            BR_BITS = 6,                                // Counter limit
            BR_LIMIT = 53,                              // Baudrate limit
            SB_TICK = 16;                               // Sb tick
reg  [3:0] rx_index;                                    // Rx module index
reg  [3:0] tx_index;                                   // Tx module index
reg  [1:0] str_index;                                   // Stirng index
/* verilator lint_off WIDTH */
reg  [DBITS-1:0] rx_Data_Buffer[0:DATA_LENGTH];         // Initial rx Data Buffer
reg  [DBITS-1:0] tx_Data_Buffer[0:DATA_LENGTH];      // Initial tx Data Buffer
wire [DBITS-1:0] rx_data_out;                           // Rx Data which receive from Rx module
reg  [DBITS-1:0] tx_data_in;                            // Tx Data input to transfer
reg  [DBITS-1:0] number1;                               // Number1 to calculate in Full Adder
reg  [DBITS-1:0] number2;                               // Number2 to calculate in Full Adder
reg  [DBITS-6:0] selection;                             // select the mode
wire [2*(DBITS-1):0] result;                            // Sum of calculate in Full Adder
reg cin;
/* verilator lint_off UNUSED */
// reg reset;
reg tx_Send;                                    
wire tx_done;
wire rx_done;
wire tick;
reg [2:0] rx_current_state, rx_next_state;
reg [2:0] tx_current_state, tx_next_state;
parameter [2:0] START           = 3'b000,
                DATA            = 3'b001,
                SEND            = 3'b100,
                STOP            = 3'b111;


parameter [2:0] ADD             = 3'b001,
                SUB             = 3'b010,
                MUL             = 3'b011,
                DIV             = 3'b100;
/******Main Program********/
initial begin
     cin = 1'b0;
     tx_Send = 1'b0;
     number1 = 8'd0;
     number2 = 8'd0;
     rx_current_state = 3'd0;
     tx_current_state = 3'd0;
end

integer STDERR = 32'h8000_0002;

always @(posedge clk ) begin
    case (rx_current_state)
       START : begin
        if(((rx_data_out == 8'h0C)) &&(rx_done))
        begin
            rx_next_state <= DATA;
            $fdisplay(STDERR,"sim rx data");
        end
       end
       DATA : begin
        if(((rx_data_out == 8'hFF))&&(rx_done))
        begin
            rx_next_state <= STOP;
            $fdisplay(STDERR,"sim rx stop");
        end
       end
       STOP: begin
        if((rx_done))
        begin
            rx_next_state <= START;
            $fdisplay(STDERR,"sim rx start");
        end
       end
        default: rx_next_state <= START;
    endcase
end

// Sequential logic
/* verilator lint_off SYNCASYNCNET */
always @(posedge clk) begin
    if(reset)
    begin
        rx_current_state <= 3'd0;
        tx_current_state <= 3'd0;      
    end
    else
    begin
        rx_current_state <= rx_next_state;
        tx_current_state <= tx_next_state;
    end

end
/*------Receiver Process--------*/
always @(posedge clk) begin
    if((rx_data_out == 8'hFF)&&(rx_done))
        rx_index <= 8'd0;
    else if(((rx_data_out != 8'hFF))&&(rx_done))
        rx_index <= rx_index + 1'b1;

end    

always @(posedge clk) begin
    rx_Data_Buffer[rx_index] <= rx_data_out;
end
//FSM
always @(posedge clk) begin
    case (rx_current_state)
        START : begin
            number1 <= number1;
            number2 <= number2;
        end  
        DATA : begin
            number1 <= rx_Data_Buffer[0];
            number2 <= rx_Data_Buffer[1];
            selection <= rx_Data_Buffer[2];
        end
        STOP: begin
            number1 <= number1;
            number2 <= number2;
        end
        default: begin
            number1 <= 0;
            number1 <= 0; 
        end
    endcase
end


// /*------Transfer Process--------*/	
always @(posedge clk) begin
    if((rx_current_state == STOP))
    begin
        tx_Data_Buffer[0] <= 8'h20;
        tx_Data_Buffer[1] <= number1;
        tx_Data_Buffer[2] <= number2;
        tx_Data_Buffer[3] <= selection;
        tx_Data_Buffer[4] <= result >> 8;
        tx_Data_Buffer[5] <= result & 16'h00ff;
        tx_Data_Buffer[6] <= 8'h0A;
        tx_Data_Buffer[7] <= 8'hFF; 
    end
end	

always @(posedge clk ) begin
    case (tx_current_state)
       START : begin
        if((rx_current_state==STOP))
        begin
            tx_next_state <= SEND; 
            $fdisplay(STDERR,"sim tx send");
        end
       end
       SEND : begin
        if((tx_index == 8'd8))
        begin
           tx_next_state <= STOP; 
            $fdisplay(STDERR,"sim tx stop");
        end
       end
       STOP : begin
        if((rx_current_state==START) && (rx_done == 1'b1))
        begin
            tx_next_state <= START;
            $fdisplay(STDERR,"sim tx start");
        end
       end
        default: tx_next_state <= START;
    endcase
end
always @(posedge clk) begin
    if((tx_done)&&((tx_index != 8'd8)))
        tx_index <= tx_index + 1;
    else if((tx_done)&&((tx_index == 8'd8)))
        tx_index <= 4'd0;
end
always @(posedge clk) begin
    tx_data_in <= tx_Data_Buffer[tx_index];
end
always @(posedge clk) begin
    case (tx_current_state)
        START : begin
            tx_Send <= 1'b0;
        end  
        SEND : begin
            tx_Send <= 1'b1;
        end
        STOP: begin
            tx_Send <= 1'b0;
        end
        default: begin
            tx_Send <= 1'b0;
        end
    endcase
end

/*------Connect Signal Module---*/
baud_rate_generator 
    #(
        .N(BR_BITS), 
        .M(BR_LIMIT)
     ) 
    BAUD_RATE_GEN   
    (
        .clk(clk), 
        .reset(reset),
        .tick(tick)
     );

Receiver
    #(
        .DBITS(DBITS),
        .SB_TICK(SB_TICK)
    )
    UART_RX_UNIT
    (
        .clk(clk),
        .reset(reset),
        .rx(i_uart_rx),
        .sample_tick(tick),
        .data_ready(rx_done),
        .data_out(rx_data_out)
    );
alu 
    ALU1
    (
        .clk(clk),
        .number1(number1),
        .number2(number2),
        .sel(selection),
        .alu_out(result)
    );
Transmitter
    #(
        .DBITS(DBITS),
        .SB_TICK(SB_TICK)
     )
     UART_TX_UNIT
     (
        .clk(clk),
        .reset(reset),
        .tx_start(tx_Send),
        .sample_tick(tick),
        .data_in(tx_data_in),
        .tx_done(tx_done),
        .tx(o_uart_tx)
     );
endmodule

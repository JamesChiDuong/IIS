
/* verilator lint_off UNUSED */
module TranAndRecei (
    input clk,
    input i_uart_rx,
    input reset,
    /* verilator lint_off UNDRIVEN */
    output o_uart_tx
);
localparam  DBITS = 8,                                  // 8 bit Data
            DATA_LENGTH = 11,                           // String length of the Data buffer    
            BR_BITS = 6,                                // Counter limit
            BR_LIMIT = 53,                              // Baudrate limit
            SB_TICK = 16;                               // Sb tick
reg  [3:0] rx_index;                                    // Rx module index
reg  [10:0] tx_index;                                   // Tx module index
reg  [1:0] str_index;                                   // Stirng index
/* verilator lint_off WIDTH */
reg  [DBITS-1:0] rx_Data_Buffer[0:DATA_LENGTH];         // Initial rx Data Buffer
reg  [DBITS-1:0] tx_Data_Buffer[0:DATA_LENGTH+38];      // Initial tx Data Buffer
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
reg	[27:0]	counter;
reg CHECKOK;
reg [2:0] rx_current_state, rx_next_state;
reg [2:0] tx_current_state, tx_next_state;
parameter [2:0] START           = 3'b000,
                NUMBER1         = 3'b001,
                NUMBER2         = 3'b010,
                OPERAND         = 3'b011,
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

always @(posedge clk ) begin
    case (rx_current_state)
       START : begin
        if(((rx_data_out == 8'd45)) &&(rx_done))
            rx_next_state <= NUMBER1;
       end
       NUMBER1 : begin
        if(((rx_data_out == 8'd45)) &&(rx_done))
            rx_next_state <= NUMBER2;
       end
       NUMBER2 : begin
        if(((rx_data_out == 8'd10)) &&(rx_done))
            rx_next_state <= OPERAND;
       end
       OPERAND : begin
        rx_next_state <= STOP;
       end
       STOP: begin
        if((rx_done))
            rx_next_state <= START;
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
    if((rx_data_out == 8'd0)&&(rx_done) || (rx_index > 11))
        rx_index <= 8'd0;
    else if(((rx_data_out != 8'd10) ||(rx_data_out != 8'd45) )&&(rx_done))
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
        NUMBER1 : begin
            number1 <= (rx_Data_Buffer[0] - 48) *100 + (rx_Data_Buffer[1] - 48)*10 + (rx_Data_Buffer[2] -48);
            number2 <= 0;
        end
        NUMBER2 : begin
            number1 <= number1;
            number2 <= (rx_Data_Buffer[4] - 48) *100 + (rx_Data_Buffer[5] - 48)*10 + (rx_Data_Buffer[6] -48);        
        end
        OPERAND : begin
            if((rx_Data_Buffer[8] == "a")&&(rx_Data_Buffer[9] == "d") && (rx_Data_Buffer[10] == "d"))
            begin
                selection <= ADD;
            end
            else if ((rx_Data_Buffer[8] == "s")&&(rx_Data_Buffer[9] == "u") && (rx_Data_Buffer[10] == "b"))
            begin
                selection <= SUB;
            end
            else if ((rx_Data_Buffer[8] == "m")&&(rx_Data_Buffer[9] == "u") && (rx_Data_Buffer[10] == "l"))
            begin
                selection <= MUL;
            end
            else if ((rx_Data_Buffer[8] == "d")&&(rx_Data_Buffer[9] == "i") && (rx_Data_Buffer[10] == "v"))
            begin
                selection <= DIV;
            end
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
        tx_Data_Buffer[0] <= " ";
        tx_Data_Buffer[1] <= "N";
        tx_Data_Buffer[2] <= "U";
        tx_Data_Buffer[3] <= "M";
        tx_Data_Buffer[4] <= "B";
        tx_Data_Buffer[5] <= "E";
        tx_Data_Buffer[6] <= "R";
        tx_Data_Buffer[7] <= "1";
        tx_Data_Buffer[8] <= ":";

        tx_Data_Buffer[9] <= (number1/100 + 48);
        tx_Data_Buffer[10] <= ((number1%100)/10) + 48;
        tx_Data_Buffer[11] <= ((number1%100)%10) + 48;
        tx_Data_Buffer[12] <= 8'd32;

        tx_Data_Buffer[13] <= "N";
        tx_Data_Buffer[14] <= "U";
        tx_Data_Buffer[15] <= "M";
        tx_Data_Buffer[16] <= "B";
        tx_Data_Buffer[17] <= "E";
        tx_Data_Buffer[18] <= "R";
        tx_Data_Buffer[19] <= "2";
        tx_Data_Buffer[20] <= ":";

        tx_Data_Buffer[21] <= (number2/100 + 48);
        tx_Data_Buffer[22] <= ((number2%100)/10) + 48;
        tx_Data_Buffer[23] <= ((number2%100)%10) + 48;
        tx_Data_Buffer[24] <= 8'd32;

        tx_Data_Buffer[25] <= "O";
        tx_Data_Buffer[26] <= "P";
        tx_Data_Buffer[27] <= "E";
        tx_Data_Buffer[28] <= "R";
        tx_Data_Buffer[29] <= "A";
        tx_Data_Buffer[30] <= "N";
        tx_Data_Buffer[31] <= "D";
        tx_Data_Buffer[32] <= ":";
        tx_Data_Buffer[33] <= rx_Data_Buffer[8];
        tx_Data_Buffer[34] <= rx_Data_Buffer[9];
        tx_Data_Buffer[35] <= rx_Data_Buffer[10];
        tx_Data_Buffer[36] <= 8'd32;

        tx_Data_Buffer[37] <= "R";
        tx_Data_Buffer[38] <= "E";
        tx_Data_Buffer[39] <= "S";
        tx_Data_Buffer[40] <= "U";
        tx_Data_Buffer[41] <= "L";
        tx_Data_Buffer[42] <= "T";
        tx_Data_Buffer[43] <= ":";

        tx_Data_Buffer[44] <= (result/10000 + 48); //6
        tx_Data_Buffer[45] <= ((result%10000)/1000) + 48; //
        tx_Data_Buffer[46] <= (((result%10000)%1000)/100) + 48;
        tx_Data_Buffer[47] <= ((((result%10000)%1000)%100)/10) + 48;
        tx_Data_Buffer[48] <= ((((result%10000)%1000)%100)%10) + 48;
        tx_Data_Buffer[49] <= "\n";
    end
end	

always @(posedge clk ) begin
    case (tx_current_state)
       START : begin
        if((rx_current_state==STOP))
        begin
            tx_next_state <= SEND; 
        end
       end
       SEND : begin
        if((tx_index > 11'd49))
        begin
           tx_next_state <= STOP; 
        end
       end
       STOP : begin
        if((rx_current_state==START) && (rx_done == 1'b1))
        begin
            tx_next_state <= START;
        end
       end
        default: tx_next_state <= START;
    endcase
end
always @(posedge clk) begin
    if((tx_done)&&(tx_index < 11'd51))
        tx_index <= tx_index + 1;
    else if(tx_index >= 11'd51)
        tx_index <= 11'd0;
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
// fullAdder 
//     FA
//     (
//     .number1(number1),
//     .number2(number2),
//     .cin(cin),
//     .sum(sum),
//     .cout(cout)
//     );
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



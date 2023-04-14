`define EOF 32'hFFFF_FFFF 
`define NULL 0
`define NP 20
`define SEED 1
module fullAdder( input wire a, input wire b, input wire cin, output wire s, output wire cout);

    assign s = a ^ b ^ cin;
    assign cout = b&cin | a&b | a&cin;
endmodule
module adder_8_s(   input wire [7:0] a,
                    input wire [7:0] b,
                    input wire cin,
                    output wire [7:0] s,
                    output wire cout
);
/* The adder is build up by a cascade connection of FA's. 'c' is an
* AUXILIARY signal to connect the carry output of one stage with the carry input of the next one
*/
wire [7:0] c;

fullAdder fa0 (.a(a[0]), .b(b[0]), .cin(cin), .s(s[0]), .cout(c[1]));
fullAdder fa1 (.a(a[1]), .b(b[1]), .cin(c[1]), .s(s[1]), .cout(c[2]));
fullAdder fa2 (.a(a[2]), .b(b[2]), .cin(c[2]), .s(s[2]), .cout(c[3]));
fullAdder fa3 (.a(a[3]), .b(b[3]), .cin(c[3]), .s(s[3]), .cout(c[4]));
fullAdder fa4 (.a(a[4]), .b(b[4]), .cin(c[4]), .s(s[4]), .cout(c[5]));
fullAdder fa5 (.a(a[5]), .b(b[5]), .cin(c[5]), .s(s[5]), .cout(c[6]));
fullAdder fa6 (.a(a[6]), .b(b[6]), .cin(c[6]), .s(s[6]), .cout(c[7]));
/*The carry output of the last FA is the carry output of the adder*/
fullAdder fa7 (.a(a[7]), .b(b[7]), .cin(c[7]), .s(s[7]), .cout(cout));
endmodule    
module testbench();
/********IN/OUT variable************************/
    reg [7:0] outputNumber1; /****************Input Array*/
    reg [7:0] outputNumber2;
    reg cin = 0; /********************************Carry input*/
    wire[7:0] sum;/******************************Sum*/
    wire cout;/*******************************Carry output*/
/********PARAMETER**********************************/
    parameter STDIN = 32'h8000_0000;
    integer scan_file;

    adder_8_s uut(.a(outputNumber1),.b(outputNumber2),.cin(cin),.s(sum),.cout(cout));
    initial 
    begin
        scan_file = $fscanf(STDIN, "%d%d", outputNumber1,outputNumber2);
        #10;
        $display("%b %b %d %b %d",outputNumber1,outputNumber2,cin,sum,cout);
    end
endmodule
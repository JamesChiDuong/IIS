#include <verilatedos.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "verilated.h"
#ifdef	USE_UART_LITE
#include "VTranAndReceilite.h"
#define	SIMCLASS	VTranAndReceilite
#else
#include "VTranAndRecei.h"
#define	SIMCLASS	VTranAndRecei
#endif
#include "verilated_vcd_c.h"
#include "uartsim.h"

#define WAVENAME "TranAndRecei.vcd"
int	main(int argc, char **argv) 
{
   Verilated::commandArgs(argc, argv);
   UARTSIM		*uart;										// init uart pointer
                                                   //uart_PseudoTerminal Pseudo;							// Init Pseudo terminal
   unsigned	setup = 868;								// init baudrate

   // Setup the model and baud rate
   SIMCLASS tb;
   int baudclocks = setup & 0x0ffffff;
   tb.i_uart_rx = 1;

   uart = new UARTSIM();
   uart->setup(setup);

   // Make sure we don't run longer than 4 seconds ...
   unsigned	clocks = 0;

   // VCD trace setup
#define	VCDTRACE
#ifdef	VCDTRACE
   Verilated::traceEverOn(true);
   VerilatedVcdC* tfp = new VerilatedVcdC;
   tb.trace(tfp, 99);
   tfp->open(WAVENAME);
#define	TRACE_POSEDGE	tfp->dump(10*clocks)
#define	TRACE_NEGEDGE	tfp->dump(10*clocks+5)
#define	TRACE_CLOSE	tfp->close()
#else
#define	TRACE_POSEDGE	while(0)
#define	TRACE_NEGEDGE	while(0)
#define	TRACE_CLOSE	while(0)
#endif

   // Clear any initial break condition
   for(int i=0; i<(baudclocks*24); i++) {
      tb.clk = 1;
      tb.eval();
      tb.clk = 0;
      tb.eval();

      tb.i_uart_rx = 1;
   }

   // Simulation loop: process the hello world string
   while(1)
   {
      tb.clk = 1;
      tb.eval();
      TRACE_POSEDGE;
      tb.clk = 0;
      tb.eval();
      TRACE_NEGEDGE;
      clocks++;
      tb.i_uart_rx = (*uart)(tb.o_uart_tx);
   }
   TRACE_CLOSE;

   exit(EXIT_SUCCESS);
}


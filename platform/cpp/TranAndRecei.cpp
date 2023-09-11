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
#include <iostream>
#include <cstring>
#ifdef	USE_UART_LITE
#include "VTranAndReceilite.h"
#define	SIMCLASS	VTranAndReceilite
#else
#include "VTranAndRecei.h"
#define	SIMCLASS	VTranAndRecei
#endif
#include "verilated_vcd_c.h"
#include "uartsim.h"

#define LENG 10
using namespace std;

#define WAVENAME "TranAndRecei.vcd"
int	main(int argc, char **argv) 
{
   Verilated::commandArgs(argc, argv);
   UARTSIM		*uart;										// init uart pointer
   uart_PseudoTerminal *Pseudo;							// Init Pseudo terminal
   bool		run_interactively = false;
   int		port = 0;										// Port is zero mean transfer through standard input and output
   unsigned	setup = 868;								// init baudrate
   int fd;

   int	childs_stdin[2], childs_stdout[2];
   if ((pipe(childs_stdin)!=0)||(pipe(childs_stdout) != 0)) 
   {
      fprintf(stderr, "ERR setting up child pipes\n");
      perror("O/S ERR");
      printf("TEST FAILURE\n");
      exit(EXIT_FAILURE);
   }

   // Set up first child process
   pid_t childs_pid = fork();

   if (childs_pid != 0) 
   { // The parent, feeding the simulation

      while (1) // re-open pseudo terminal if closed by client
      {
         int	nr=1, nw;

         // We are the parent
         //close(childs_stdin[ 0]); // Close the read end
         //close(childs_stdout[1]); // Close the write end

         Pseudo->PseudoTerminal_Init(&fd);						// Init Pesudo Terminal

         pid_t childs_pid = fork();
         if (childs_pid != 0) 
         { // the parent - forward pseudo terminal to simulation
            char Buffer; //[100];

            while (1)
            {
               if (Pseudo->PseudoTerminal_readByte(fd, &Buffer) < 0)
               {
                  fprintf(stderr, "error read\n");
                  break;
               }

               write(childs_stdin[1], &Buffer, 1);
            }

            kill(childs_pid, SIGTERM);

            sleep(1);

            Pseudo->PseudoTerminal_Deinit(fd);
         }
         else
         { // the child
            char Buffer[100];

            while(1)
            {
               char a;
               read(childs_stdout[0],&a, 1);

               Pseudo->PseudoTerminal_writeByte(fd,a);
            }
         }
      }
   }
   else 
   { // The child (Verilator simulation)

      // Fix up the FILE I/O
      close(childs_stdin[ 1]);
      close(childs_stdout[0]);
      close(STDIN_FILENO);
      if (dup(childs_stdin[0]) < 0) 
      {
         fprintf(stderr, "ERR setting up child FD\n");
         perror("O/S ERR");
         exit(EXIT_FAILURE);
      }
      close(STDOUT_FILENO); 
      if (dup(childs_stdout[1]) < 0) 
      {
         fprintf(stderr, "ERR setting up child FD\n");
         perror("O/S ERR");
         exit(EXIT_FAILURE);
      }

      // Setup the model and baud rate
      SIMCLASS tb;
      int baudclocks = setup & 0x0ffffff;
      tb.i_uart_rx = 1;

      // UARTSIM(0) uses stdin and stdout for its FD's
      uart = new UARTSIM(0);
      uart->setup(setup);

      // Make sure we don't run longer than 4 seconds ...
      time_t	start = time(NULL);
      int	iterations_before_check = 2048;
      unsigned	clocks = 0;
      bool	done = false;

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
}


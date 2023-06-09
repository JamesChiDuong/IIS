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
#include "VData_Receiverlite.h"
#define	SIMCLASS	VData_Receiverlite
#else
#include "VData_Receiver.h"
#define	SIMCLASS	VData_Receiver
#endif
#include "verilated_vcd_c.h"
#include "uartsim.h"

int	main(int argc, char **argv) 
{
	Verilated::commandArgs(argc, argv);
	UARTSIM		*uart;									// Init Uart Simulate
	uart_PseudoTerminal *Pseudo;						// Init the Pseudo Terminal pointer
	bool		run_interactively = false;
	int		port = 0;
	unsigned	setup = 868;							// Init the baudrate
	int fd;
	Pseudo->PseudoTerminal_Init(&fd);					// Setting the parameter of Pseudo
	// Argument processing
	// {{{
	for(int argn=1; argn<argc; argn++) 
    {
		if (argv[argn][0] == '-') for(int j=1; (j<1000)&&(argv[argn][j]); j++)
		switch(argv[argn][j]) 
        {
			case 'i':
				run_interactively = true;
				break;
			case 'p':
				port = atoi(argv[argn++]); j+= 4000;
				run_interactively = true;
				break;
			case 's':
				setup= strtoul(argv[++argn], NULL, 0); j+= 4000;
				break;
			default:
				printf("Undefined option, -%c\n", argv[argn][j]);
				break;
		}
	}
	// }}}

	if (run_interactively) 								// if using the option -i or -p
    {
		// Setup the model and baud rate
		// {{{
		SIMCLASS tb;									// declerate the tb variable
		tb.i_uart_rx = 1;
		// }}}	


		// {{{
		uart = new UARTSIM(port);						// Init the UART sim ith the port is the standard input
		uart->setup(setup);								// Set up the parameter

		while(1) 
        {

			tb.clk = 1;
			tb.eval();
			tb.clk = 0;
			tb.eval();
			tb.i_uart_rx = (*uart)(tb.o_uart_tx);		// transfer and receiver via tx and rx
		}
		// }}}
	} else 
    {
		// Set up a child process
		// {{{
		char Buffer[30];
		int	childs_stdin[2], childs_stdout[2];
		Pseudo->PseudoTerminal_readData(fd,Buffer);		// Read data and store into the Buffer
		

		if ((pipe(childs_stdin)!=0)||(pipe(childs_stdout) != 0)) 
        {
			fprintf(stderr, "ERR setting up child pipes\n");
			perror("O/S ERR");
			printf("TEST FAILURE\n");
			exit(EXIT_FAILURE);
		}

		pid_t childs_pid = fork();						// inithe the child

		if (childs_pid < 0) 
        {
			fprintf(stderr, "ERR setting up child process\n");
			perror("O/S ERR");
			printf("TEST FAILURE\n");
			exit(EXIT_FAILURE);
		}
		// }}}

		if (childs_pid) 
        { // The parent, feeding the simulation
			// {{{
			int	nr=-2, nw;

			// We are the parent
			close(childs_stdin[ 0]); // Close the read end
			close(childs_stdout[1]); // Close the write end

			char test[256];

			nw = write(childs_stdin[1], Buffer, strlen(Buffer));		// write into the childs_stdin with the buffer
			if (nw == (int)strlen(Buffer)) 								// check if read is the same the received data
            {
				int	rpos = 0;
				test[0] = '\0';
				while((rpos<nw)											// check untill get the enough the buffer
					&&(0<(nr=read(childs_stdout[0],
						&test[rpos], (strlen(Buffer))-rpos))))
					rpos += nr;
				
				nr = rpos;
				if (rpos > 0)
					test[rpos] = '\0';
				printf("Successfully read %d characters: %s\n", nr, test);
				Pseudo->PseudoTerminal_writeData(fd,test);				// Send back the String
				Pseudo->PseudoTerminal_Deinit(fd);						// close
			}

			int	status = 0, rv = -1;

			// Give the child the oppoortunity to take another
			// 60 seconds to finish closing itself
			for(int waitcount=0; waitcount < 60; waitcount++) 			// Waitting to check
            {
				rv = waitpid(-1, &status, WNOHANG);
				if (rv == childs_pid)
					break;
				else if (rv < 0)
					break;
				else // rv == 0
					sleep(1);
			}

			if (rv != childs_pid) 
            {
				kill(childs_pid, SIGTERM);
				printf("WARNING: Child/simulator did not terminate normally\n");
			}

			if (WEXITSTATUS(status) != EXIT_SUCCESS) 
            {
				printf("WARNING: Child/simulator exit status does not indicate success\n");
			}

			if ((nr == nw)&&(nw == (int)strlen(Buffer))						// if the input string == the output string it will pass
					&&(strcmp(test, Buffer) == 0)) 
            {
				printf("PASS!\n");
				exit(EXIT_SUCCESS);
			} else 
            {
				printf("TEST FAILED\n");
				exit(EXIT_FAILURE);
			}
			// }}}
		} else 
        { // The child (Verilator simulation)
			// {{{

			// Fix up the FILE I/O
			// {{{

			close(childs_stdin[ 1]);										// Close stdin
			close(childs_stdout[0]);										// Close stdout
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
			// {{{
			SIMCLASS tb;
			int baudclocks = setup & 0x0ffffff;
			tb.i_uart_rx = 1;
			// }}}

			// UARTSIM(0) uses stdin and stdout for its FD's
			uart = new UARTSIM(0);
			uart->setup(setup);
			// }}}

			// Make sure we don't run longer than 4 seconds ...
			time_t	start = time(NULL);
			int	iterations_before_check = 2048;
			unsigned	clocks = 0;
			bool	done = false;

			// VCD trace setup
			// {{{
#define	VCDTRACE
#ifdef	VCDTRACE
			Verilated::traceEverOn(true);
			VerilatedVcdC* tfp = new VerilatedVcdC;
			tb.trace(tfp, 99);
			tfp->open("Data_Receiver.vcd");
#define	TRACE_POSEDGE	tfp->dump(10*clocks)
#define	TRACE_NEGEDGE	tfp->dump(10*clocks+5)
#define	TRACE_CLOSE	tfp->close()
#else
#define	TRACE_POSEDGE	while(0)
#define	TRACE_NEGEDGE	while(0)
#define	TRACE_CLOSE	while(0)
#endif
			// }}}

			// Clear any initial break condition
			// {{{
			for(int i=0; i<(baudclocks*24); i++) {
				tb.clk = 1;
				tb.eval();
				tb.clk = 0;
				tb.eval();

				tb.i_uart_rx = 1;
			}
			// }}}

			// Simulation loop: process the hello world string
			// {{{
			while(clocks < 2*(baudclocks*16)*strlen(Buffer)) {
				tb.clk = 1;
				tb.eval();
				TRACE_POSEDGE;
				tb.clk = 0;
				tb.eval();
				TRACE_NEGEDGE;
				clocks++;

				tb.i_uart_rx = (*uart)(tb.o_uart_tx);				// the cpp will receive data from python and then transfer data to verilog file. 
																	// The verilog file will transfer back the simulation file after recive data
																	// The cpp will transfer data to the python back again.

				if (iterations_before_check-- <= 0) {
					iterations_before_check = 2048;
					done = ((time(NULL)-start)>60);
					if (done)
					fprintf(stderr, "CHILD-TIMEOUT\n");
				}
			}
			// }}}

			TRACE_CLOSE;

			exit(EXIT_SUCCESS);
			// }}}
		}
	}
}
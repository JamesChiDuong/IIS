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
char* GetSubString(char* str, int index, int count) {
	int strLen = strlen(str);
	int lastIndex = index + count;

	if (index >= 0 && lastIndex > strLen) return "";

	char* subStr = (char*)(malloc(count + 1));

	for (int i = 0; i < count; i++) {
		subStr[i] = str[index + i];
	}

	subStr[count] = '\0';

	return subStr;
}

char* AppendString(const char* str1, const char* str2) {
	int str1Len = strlen(str1);
	int str2Len = strlen(str2);
	int strLen = str1Len + str2Len + 1;
	char* str = (char*)malloc(strLen);

	for (int i = 0; i < str1Len; i++)
		str[i] = str1[i];

	for (int i = 0; i < str2Len; i++)
		str[(str1Len + i)] = str2[i];

	str[strLen - 1] = '\0';

	return str;
}

char* InsertString(char* str, int index, char* subStr) {
	char* s = GetSubString(str, 0, index);
	s = AppendString(s, subStr);
	s = AppendString(s, GetSubString(str, index, strlen(str) - index));

	return s;
}

char* CharToString(char c) {
	char* str = (char*)malloc(2);
	str[0] = c;
	str[1] = '\0';

	return str;
}

char* DecimalToHexadecimal(int dec)
{
	if (dec < 1) return "0";

	int hex = dec;
	char* hexStr = "";

	while (dec > 0)
	{
		hex = dec % 16;

		if (hex < 10)
			hexStr = InsertString(hexStr, 0, CharToString(hex + 48));
		else
			hexStr = InsertString(hexStr, 0, CharToString(hex + 55));

		dec /= 16;
	}

	return hexStr;
}

char* ASCIIToHexadecimal(char* str) {
	char* hex = "";
	int strLen = strlen(str);

	for (int i = 0; i < strLen; ++i)
	{
		char* chex = DecimalToHexadecimal(str[i]);

		if (strlen(chex) == 1)
			chex = InsertString(chex, 0, "0");

		hex = AppendString(hex, chex);
	}

	return hex;
}
int	main(int argc, char **argv) 
{
    Verilated::commandArgs(argc, argv);
    UARTSIM		*uart;										// init uart pointer
    uart_PseudoTerminal *Pseudo;							// Init Pseudo terminal
    bool		run_interactively = false;
    int		port = 0;										// Port is zero mean transfer through standard input and output
    unsigned	setup = 868;								// init baudrate
    int fd;

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

    if (run_interactively) 										// if option is -i or -p
    {
        // Setup the model and baud rate
        // {{{
        SIMCLASS tb;
        tb.i_uart_rx = 1;
        // }}}


        // {{{
        uart = new UARTSIM(port);								// Init uart port
        uart->setup(setup);
        unsigned	clocks = 0;										// Setting uart
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

    while(1) { // we need to using the clock with the total data
        tb.clk = 1;
        tb.eval();
        TRACE_POSEDGE;
        tb.clk = 0;
        tb.eval();
        TRACE_NEGEDGE;
        clocks++;
        (tb.i_uart_rx) = (*uart)(tb.o_uart_tx);

    }
        // }}}
    } else 
    {
        // Set up a child process
        // {{{

        while(1)
        {
            Pseudo->PseudoTerminal_Init(&fd);						// Init Pesudo Terminal
            char Buffer[100];
            int	childs_stdin[2], childs_stdout[2];
            if ((pipe(childs_stdin)!=0)||(pipe(childs_stdout) != 0)) 
            {
                fprintf(stderr, "ERR setting up child pipes\n");
                perror("O/S ERR");
                printf("TEST FAILURE\n");
                exit(EXIT_FAILURE);
            }
            Pseudo->PseudoTerminal_readData(fd,Buffer);				// Read data and get Data Buffer
            pid_t childs_pid = fork();

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
                int	nr=1, nw;

                // We are the parent
                close(childs_stdin[ 0]); // Close the read end
                close(childs_stdout[1]); // Close the write end

                char test[50];
                char testData[50];
                nw = write(childs_stdin[1], Buffer, (strlen(Buffer)+LENG)); // Write data mor than string length because we transfer 2 number and want to get more than 2 number. example- input: 110 120, output: NUMBER1:112 NUMBER2:190 SUM:046 COUT:1
                if (nw == ((int)strlen(Buffer) + LENG)) 
                {
                    int	rpos = 0;
                    test[0] = '\0';
                    while((rpos<nw)&&(0<(nr)))
                    {
                        nr=read(childs_stdout[0],&test[rpos], (strlen(Buffer)+LENG)-rpos);
                        rpos += nr;

                    }
                    nr = rpos;
                    if (rpos > 0)
                        test[rpos] = '\0';  
                    
                    printf("Successfully read %d characters: %s\n", (nr-9), test);
                   
                    //strcpy(testData,test);
                    Pseudo->PseudoTerminal_writeData(fd,test);
                    sleep(1);
                    Pseudo->PseudoTerminal_Deinit(fd);
                }
                int	status = 0, rv = -1;

                // Give the child the oppoortunity to take another
                // 60 seconds to finish closing itself
                for(int waitcount=0; waitcount < 600; waitcount++) 
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

                if ((nw == ((int)strlen(Buffer) + LENG))) 
                {
                    printf("PASS!\n");
                    
                    //exit(EXIT_SUCCESS);
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
                tfp->open(WAVENAME);
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
                while(clocks < 3*(baudclocks*16)*(strlen(Buffer)+LENG)) { // we need to using the clock with the total data
                    tb.clk = 1;
                    tb.eval();
                    TRACE_POSEDGE;
                    tb.clk = 0;
                    tb.eval();
                    TRACE_NEGEDGE;
                    clocks++;
                    tb.i_uart_rx = (*uart)(tb.o_uart_tx);

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
}
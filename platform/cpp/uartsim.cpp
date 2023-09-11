////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	uartsim.cpp
// {{{
// Project:	wbuart32, a full featured UART with simulator
//
// Purpose:	To forward a Verilator simulated UART link over a TCP/IP pipe.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
// }}}
// Copyright (C) 2015-2022, Gisselquist Technology, LLC
// {{{
// This program is free software (firmware): you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program.  (It's in the $(ROOT)/doc directory.  Run make with no
// target there if the PDF file isn't present.)  If not, see
// <http://www.gnu.org/licenses/> for a copy.
// }}}
// License:	GPL, v3, as defined and found on www.gnu.org,
// {{{
//		http://www.gnu.org/licenses/gpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
//
// }}}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <ctype.h>

#include "uartsim.h"

// setup_listener
// {{{
void	UARTSIM::setup_listener(const int port) {
	struct	sockaddr_in	my_addr;

	signal(SIGPIPE, SIG_IGN);

	printf("Listening on port %d\n", port);

	m_skt = socket(AF_INET, SOCK_STREAM, 0);
	if (m_skt < 0) {
		perror("ERR: Could not allocate socket: ");
		exit(EXIT_FAILURE);
	}

	// Set the reuse address option
	{
		int optv = 1, er;
		er = setsockopt(m_skt, SOL_SOCKET, SO_REUSEADDR, &optv, sizeof(optv));
		if (er != 0) {
			perror("ERR: SockOpt Err:");
			exit(EXIT_FAILURE);
		}
	}

	memset(&my_addr, 0, sizeof(struct sockaddr_in)); // clear structure
	my_addr.sin_family = AF_INET;
	// Use *all* internet ports to this computer, allowing connections from
	// any/every one of them.
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	my_addr.sin_port = htons(port);

	if (bind(m_skt, (struct sockaddr *)&my_addr, sizeof(my_addr))!=0) {
		perror("ERR: BIND FAILED:");
		exit(EXIT_FAILURE);
	}

	if (listen(m_skt, 1) != 0) {
		perror("ERR: Listen failed:");
		exit(EXIT_FAILURE);
	}
}
// }}}

// UARTSIM::UARTSIM(const int port)
// {{{
UARTSIM::UARTSIM(const int port) {
	m_conrd = m_conwr = m_skt = -1;
	if (port == 0) {
		m_conrd = STDIN_FILENO;
		m_conwr = STDOUT_FILENO;
	} else
		setup_listener(port);
	setup(25);	// Set us up for (default) 8N1 w/ a baud rate of CLK/25
	m_rx_baudcounter = 0;
	m_tx_baudcounter = 0;
	m_rx_state = RXIDLE;
	m_tx_state = TXIDLE;
}
// }}}

// UARTSIM::kill
// {{{
void	UARTSIM::kill(void) {
	fflush(stdout);

	// Quickly double check that we aren't about to close stdin/stdout
	if (m_conrd == STDIN_FILENO)
		m_conrd = -1;
	if (m_conwr == STDOUT_FILENO)
		m_conwr = -1;
	// Close any active connection
	if (m_conrd >= 0)				close(m_conrd);
	if ((m_conwr >= 0)&&(m_conwr != m_conrd))	close(m_conwr);
	if (m_skt >= 0) close(m_skt);

	m_conrd = m_conwr = m_skt = -1;
}
// }}}

// UARTSIM::setup(isetup)
// {{{
void	UARTSIM::setup(unsigned isetup) {
	if (isetup != m_setup) {
		m_setup = isetup;
		m_baud_counts = (isetup & 0x0ffffff);
		m_nbits   = 8-((isetup >> 28)&0x03);
		m_nstop   =((isetup >> 27)&1)+1;
		m_nparity = (isetup >> 26)&1;
		m_fixdp   = (isetup >> 25)&1;
		m_evenp   = (isetup >> 24)&1;
	}
}
// }}}

// UARTSIM::check_for_new_connections
// {{{
void	UARTSIM::check_for_new_connections(void) {
	if ((m_conrd < 0)&&(m_conwr<0)&&(m_skt>=0)) {
		// Can we accept a connection?
		struct	pollfd	pb;

		pb.fd = m_skt;
		pb.events = POLLIN;
		poll(&pb, 1, 0);

		if (pb.revents & POLLIN) {
			m_conrd = accept(m_skt, 0, 0);
			m_conwr = m_conrd;

			if (m_conrd < 0)
				perror("Accept failed:");
			// else printf("New connection accepted!\n");
		}
	}

}
// }}}

// UARTSIM::rawtick(i_tx, network)
// {{{
int	UARTSIM::rawtick(const int i_tx, const bool network) {
	int	o_rx = 1, nr = 0;

	if (network)
		check_for_new_connections();

	if ((!i_tx)&&(m_last_tx))
		m_rx_changectr = 0;
	else	m_rx_changectr++;
	m_last_tx = i_tx;

	if (m_rx_state == RXIDLE) {
		if (!i_tx) {
			m_rx_state = RXDATA;
			m_rx_baudcounter =m_baud_counts+m_baud_counts/2-1;
			m_rx_baudcounter -= m_rx_changectr;
			m_rx_busy    = 0;
			m_rx_data    = 0;
		}
	} else if (m_rx_baudcounter <= 0) {
		if (m_rx_busy >= (1<<(m_nbits+m_nparity+m_nstop-1))) {
			m_rx_state = RXIDLE;
			if (m_conwr >= 0) 
			{
				char buf[1];
				buf[0] = (m_rx_data >> (32-m_nbits-m_nstop-m_nparity))&0x0ff;
				if ((network)&&(1 != send(m_conwr, buf, 1, 0))) {
					close(m_conwr);
					m_conrd = m_conwr = -1;
					fprintf(stderr, "Failed write, connection closed\n");
				} else if ((!network)&&(1 != write(m_conwr, buf, 1))) {
					fprintf(stderr, "ERR while attempting to write out--closing output port\n");
					perror("UARTSIM::write() ");
					m_conrd = m_conwr = -1;
				}
			}
		} else {
			m_rx_busy = (m_rx_busy << 1)|1;
			// Low order bit is transmitted first, in this
			// order:
			//	Start bit (1'b1)
			//	bit 0
			//	bit 1
			//	bit 2
			//	...
			//	bit N-1
			//	(possible parity bit)
			//	stop bit
			//	(possible secondary stop bit)
			m_rx_data = ((i_tx&1)<<31) | (m_rx_data>>1);
		} m_rx_baudcounter = m_baud_counts-1;
	} else
		m_rx_baudcounter--;

	if ((m_tx_state == TXIDLE)&&((network)||(m_conrd >= 0))) {
		struct	pollfd	pb;
		pb.fd = m_conrd;
		pb.events = POLLIN;
		if (poll(&pb, 1, 0) < 0)
			perror("Polling error:");

		if (pb.revents & POLLIN) {
			char	buf[1];

			if (network)
				nr = recv(m_conrd, buf, 1, MSG_DONTWAIT);
			else
				nr = read(m_conrd, buf, 1);
			if (1 == nr) {
				m_tx_data = (-1<<(m_nbits+m_nparity+1))
					// << nstart_bits
					|((buf[0]<<1)&0x01fe);
				if (m_nparity) {
					int	p;

					// If m_nparity is set, we need to then
					// create the parity bit.
					if (m_fixdp)
						p = m_evenp;
					else {
						p = (m_tx_data >> 1)&0x0ff;
						p = p ^ (p>>4);
						p = p ^ (p>>2);
						p = p ^ (p>>1);
						p &= 1;
						p ^= m_evenp;
					}
					m_tx_data |= (p<<(m_nbits+m_nparity));
				}
				m_tx_busy = (1<<(m_nbits+m_nparity+m_nstop+1))-1;
				m_tx_state = TXDATA;
				o_rx = 0;
				m_tx_baudcounter = m_baud_counts-1;
			} else if ((network)&&(nr == 0)) {
				close(m_conrd);
				m_conrd = m_conwr = -1;
				// printf("Closing network connection\n");
			} else if (nr < 0) {
				if (!network) {
					fprintf(stderr, "ERR while attempting to read in--closing input port\n");
					perror("UARTSIM::read() ");
					m_conrd = -1;
				} else {
					perror("O/S Read err:");
					close(m_conrd);
					m_conrd = m_conwr = -1;
				}
			}
		}
	} else if (m_tx_baudcounter <= 0) {
		m_tx_data >>= 1;
		m_tx_busy >>= 1;
		if (!m_tx_busy)
			m_tx_state = TXIDLE;
		else
			m_tx_baudcounter = m_baud_counts-1;
		o_rx = m_tx_data&1;
	} else {
		m_tx_baudcounter--;
		o_rx = m_tx_data&1;
	}

	return o_rx;
}
// }}}

// UARTSIM::nettick
// {{{
int	UARTSIM::nettick(const int i_tx) {
	return rawtick(i_tx, true);
}
// }}}

// UARTSIM::fdtick
// {{{
int	UARTSIM::fdtick(const int i_tx) {
	return rawtick(i_tx, false);
}
//Pseudo_Terminal::Init
void uart_PseudoTerminal::PseudoTerminal_Init(int *fd)
{
    	//int fd;
    struct termios tty;

    *fd = open("/dev/ptmx", O_RDWR ); 		//Open the port with ptmx
    if (*fd < 0) {
        perror("open");
    }

    grantpt(*fd); 							//To access to the slave pseudoterminal
    unlockpt(*fd);							//After the file desciptor is passed to unlockpt() to unlock the slave side

    char* slave_name = ptsname(*fd);		//In order to be able to open the slave side
    printf("Slave device: %s\n", slave_name);
	
	tcgetattr(*fd, &tty);					//Save the tty setting
    cfmakeraw(&tty);						//Set raw mode on the slave side of the PTY
	
	tty.c_cflag &= ~PARENB; 				// Clear parity bit
	tty.c_cflag &= ~CSTOPB; 				// Clear stop field, only one stop bit used in communication (most common)
	tty.c_cflag &= ~CSIZE; 					// Clear all the size bits, then use one of the statements below
	tty.c_cflag |= CS8; 					// 8 bits per byte (most common)
	tty.c_cflag &= ~CRTSCTS; 				// Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CLOCAL | CREAD;			// Turn on READ & ignore ctrl lines (CLOCAL = 1)
	/****UNIX systems provide two basic modes of input, 
	 * canonical and non-canonical mode. In canonical mode, 
	 * input is processed when a new line character is received. 
	 * The receiving application receives that data line-by-line. 
	 * This is usually undesirable when dealing with a serial port, 
	 * and so we normally want to disable canonical mode.***/
	tty.c_lflag &= ~ICANON;					//Disable Cannol mode
	tty.c_lflag &= ~ECHO; 					// Disable echo
	tty.c_lflag &= ~ECHOE; 					// Disable erasure
	tty.c_lflag &= ~ECHONL; 				// Disable new-line echo
	tty.c_lflag &= ~ISIG; 					// Disable interpretation of INTR, QUIT and SUSP
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
	tty.c_oflag &= ~OPOST; 					// Prevent special interpretation of output bytes (e.g. newline chars)
  	tty.c_oflag &= ~ONLCR; 					// Prevent conversion of newline to carriage return/line feed
    
	/***************************************This will make read() always wait for bytes (exactly how many is determined by VMIN), so read() could block indefinitely.*/
	tty.c_cc[VMIN] = 1;						
    tty.c_cc[VTIME] = 0;
    
	cfsetispeed(&tty, B115200);				//Set baudrate
  	cfsetospeed(&tty, B115200);				//Set baudrate

	if (tcsetattr(*fd, TCSANOW, &tty) != 0) {
	printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
  }
}
//Pseudo_Terminal::ReadData
void uart_PseudoTerminal::PseudoTerminal_readData(int fd,char* Buffer)
{
    char buf[120]; 							// the Buffer to store the Data
	int num = sizeof(buf);					// Assign the number of Data
    while (1) {
        ssize_t n = read(fd, &buf, num);	//Read Data via Peseudo
        if (n < 0) {						//If no read any data
            //perror("read");
            break;
        } else if (n == 0) {				//If read the 1 byte data
            printf("EOF\n");		
            break;
        } else {
            printf("Received %ld bytes: %.*s", n, (int) n, buf);
			num = 0;						//To read the last of the data
			strcpy(Buffer,buf);				//Coppy in to the Buffer
			//break;
        }
    } 
}

int uart_PseudoTerminal::PseudoTerminal_readByte(int fd, char *buf)
{
    ssize_t n = read(fd, buf, 1);	//Read Data via Peseudo
        if (n < 0) {						//If no read any data
            perror("read");
            return -1;
        } 

   printf("read %02x\n", *buf);

   return 0;
}
//Pseudo_Terminal::WriteData
void uart_PseudoTerminal::PseudoTerminal_writeData(int fd,char* Buffer)
{
	ssize_t n = write(fd, Buffer, strlen(Buffer)); //Write the data with the size is the length of the buffer data
	write(fd, "\n",1);						//Write the data end of line
    if (n < 0) {
        perror("write");
    } else {
        printf("Sent %ld bytes: %s\n", n, Buffer);
    }
}

int uart_PseudoTerminal::PseudoTerminal_writeByte(int fd, unsigned char Buffer)
{
   ssize_t n = write(fd, &Buffer, 1); //Write the data with the size is the length of the buffer data

   if (n < 0) {
      perror("write");
      return -1;
   }

   printf("Sent 1 byte: %02x\n", Buffer);

   return 0;
}
//Pseudo_Terminal::Deinit
void uart_PseudoTerminal::PseudoTerminal_Deinit(int fd)
{
	close(fd);
}

/*****************************************************************
**	File name: 		client.c
**	Author:			Xusheng Gao (xg2193)
**  Description:	
**
*****************************************************************/

/*********************** Includes *******************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>    		
#include <stdbool.h>
#include <signal.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <unistd.h>

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>		
#include <arpa/inet.h> 

using namespace std;

/******************* User Defines ********************************/

/******************* Function Prototype **************************/
void error(string str);
void quitHandler(int exit_code);

/******************* Main program ********************************/
int main(int argc, char** argv)
{   
    // check if we have enough argument passed into the program
    if (argc < 3) {
    	error("Did not specify address and port number.\n>Usage: ./Client <IP Address> <Port #>");
    	exit(EXIT_FAILURE);
    }
    
    // setup to capture process terminate signals
	signal(SIGINT, quitHandler);
	signal(SIGTERM, quitHandler);
	signal(SIGKILL, quitHandler);
	signal(SIGQUIT, quitHandler);
	signal(SIGTSTP, quitHandler);
    
    // setup for the socket
    memset(&server_addr, 0, sizeof(server_addr));
    addr = argv[1];
	port_number = atoi(argv[2]);
	
	exit(EXIT_SUCCESS);
}

/**************************************************************/
/*	quitHandler - capture process quit/stop/termination signals
/* 				  and exit gracefully.
/**************************************************************/
void quitHandler(int signal_code)
{
	shutdown(client_socket, SHUT_RDWR);
	cout << endl << ">User terminated client process." << endl;	
	exit(EXIT_SUCCESS);
}


/**************************************************************/
/*	error - Error msg helper function
/**************************************************************/
void error(string str)
{
	cout << "ERROR: " << str << endl;
}

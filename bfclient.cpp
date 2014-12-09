/*****************************************************************
**	File name: 		bfclient.c
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
#include <math.h>
#include <map>
#include <ctime>
#include <cmath>
#include <climits>

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>	
#include <netinet/udp.h>	
#include <arpa/inet.h> 
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <pthread.h> 

using namespace std;

/******************* User Defines ********************************/
#define MS_PER_SECOND							1000
#define INF										UINT_MAX

typedef struct bfpath_t
{
	uint16_t dest_port;
	string dest;
	uint16_t hop_port;
	string hop;
	double origin_cost;
	double cost;
	bool linked;
}bfpath;

typedef struct bf_packet_t
{
	uint32_t dest_ip;
	uint16_t dest_port;
	uint32_t hop_ip;
	uint16_t hop_port;
	uint16_t cost_int;
	uint16_t cost_deci;
}bf_packet;

map<string, bfpath> routing_table;

/******************* Function Prototype **************************/
void error(string str);
void quitHandler(int exit_code);
string get_time_stamp(void);
string get_own_ip(void);
void print_routing_table(void);

/******************* Main program ********************************/
int main(int argc, char* argv[])
{   
	// setup to capture process terminate signals
	signal(SIGINT, quitHandler);
	signal(SIGTERM, quitHandler);
	signal(SIGKILL, quitHandler);
	signal(SIGQUIT, quitHandler);
	signal(SIGTSTP, quitHandler);
	
    // check if we have correct number of argument pass in.
    if(argc%3 != 0 || argc == 0) 
    {
    	error("Invalid number of argument.\n>Usage: ./bfclient <local port> <ip1> <port1> <weight1> ...");
    }
    
    int localport = atoi(argv[0]);
    int timeout = atoi(argv[1]);
    string my_ip = get_own_ip();
    cout << ">My ip address is: " << my_ip << endl;
    
    if(localport < 0 || timeout < 0)
    {
    	error("Invalid argument. Argument must be greater than 0.");
    }
    
    int i = 3;
    
    while(i < argc)
    {
    	bfpath new_node;
    	
    	string ip(argv[i]);
    	string port(argv[i+1]);
    	
    	new_node.dest_port = atoi(argv[i+1]);
    	new_node.dest = ip;
    	new_node.hop_port = new_node.dest_port;
    	new_node.hop = new_node.dest;
    	new_node.cost = atof(argv[i+2]);
    	new_node.origin_cost = new_node.cost;
    	new_node.linked = false;
    	
    	routing_table[(ip + ":" + port)] = new_node;
    	i+=3;
    }
    
    print_routing_table();

	exit(EXIT_SUCCESS);
}

/**************************************************************/
/*	quitHandler - capture process quit/stop/termination signals
/* 				  and exit gracefully.
/**************************************************************/
void quitHandler(int signal_code)
{
	cout << endl << ">User terminated client process." << endl;	
	exit(EXIT_SUCCESS);
}

/**************************************************************/
/*	error - Error msg helper function
/**************************************************************/
void error(string str)
{
	cout << ">ERROR: " << str << endl;
	exit(EXIT_FAILURE);
}

/**************************************************************/
/*	BF - bellman ford
/**************************************************************/
void BF(string source) 
{
	int i, j;

	for (i = 0; i < n; ++i)
		d[i] = INFINITY;

	d[s] = 0;
	
	for(map<string, bfpath>::iterator itr=routing_table.begin(); itr!=routing_table.end(); ++itr)
	{
		string key = ((*itr).first);
		bfpath val = routing_table[key];
		
		
	}

	for (i = 0; i < routing_table.size() - 1; ++i)
	{
		for (j = 0; j < e; ++j)
		{
			if (d[edges[j].u] + edges[j].w < d[edges[j].v])
				d[edges[j].v] = d[edges[j].u] + edges[j].w;
		}
	}
}

/**************************************************************/
/*	get_own_ip - return the ip of current machine
/**************************************************************/
string get_own_ip(void)
{
	struct ifaddrs *ifaddr, *ifa;
	char my_ip[NI_MAXHOST];
	
	if (getifaddrs(&ifaddr) == -1) 
	{
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
	{
		if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET || strcmp(ifa->ifa_name, "lo") == 0)
			continue;

		if(getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), my_ip, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) != 0) 
		{
			error("getnameinfo() failed.");
		}
	}
	
	freeifaddrs(ifaddr);
	return my_ip;
}

/**************************************************************/
/*	get_time_stamp - get a time stamp in a nice formmat
/**************************************************************/
string get_time_stamp(void)
{
	char buff[10];
	time_t current_time;
	time(&current_time);
	strftime(buff, 10, "%H:%M:%S", localtime(&current_time));
	return buff;
}

/**************************************************************/
/*	print_routing_table - print out the current routing table
/**************************************************************/
void print_routing_table(void)
{
	cout << endl << get_time_stamp() << " Distance vector list is:" << endl;
	
	for(map<string, bfpath>::iterator itr=routing_table.begin(); itr!=routing_table.end(); ++itr)
	{
		string key = ((*itr).first);
		bfpath val = routing_table[key];
		
		if(val.linked)
		{
			cout << "Destination = " << key.c_str() << ", ";
			cout << "Cost = " << val.cost << ", ";
			cout << "Link = (" << val.hop << ":" << val.hop_port << ")" << endl;
		}
	}
	
	cout << endl;
}

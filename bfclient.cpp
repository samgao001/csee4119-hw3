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
#define INF										0xFFFF
#define COST_DIVIDER							50000
#define MAX_BUFF_SIZE							1024

typedef struct bfpath_t
{
	uint16_t source_port;
	uint16_t dest_port;
	string dest;
	uint16_t hop_port;
	string hop;
	double origin_cost;
	double cost;
	bool linked;
	int timeout_count;
}bfpath;

typedef struct bf_packet_t
{
	uint32_t dest_ip;
	uint16_t dest_port;
	uint16_t cost_int;
	uint16_t cost_deci;
}bf_packet;

map<string, bfpath> neighbors;
map<string, bfpath> RT;
struct timeval timeout;
string myip;
string myaddr;

/**************************************************************/
/*	itos - helper function to convert integer to string
/**************************************************************/
template <typename T>
string itos(T Number)
{
	stringstream ss;
	ss << Number;
	return ss.str();
}

/******************* Function Prototype **************************/
void error(string str);
void quitHandler(int exit_code);
string get_time_stamp(void);
string get_own_ip(void);
void BF(bf_packet* route_info, string s_ip, uint16_t s_port, uint8_t count);
void print_RT(void);

void* listening_handler(void*);
void* broadcast_handler(void*);

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
    
    uint16_t localport;
    
    localport = atoi(argv[1]);
    timeout.tv_sec = atoi(argv[2]);
    timeout.tv_usec = 0;
    myip = get_own_ip();
    myaddr = myip + ":" + itos(localport);
    cout << ">My ip address is: " << myip << endl;

    /*struct in_addr addr;
    inet_aton(my_ip.c_str(), &addr);
    cout << addr.s_addr << endl;
    
    struct in_addr test;
    test.s_addr = 251789322;
    cout << inet_ntoa(test) << endl;*/	
    
    if(localport < 0 || timeout.tv_sec < 0)
    {
    	error("Invalid argument. Argument must be greater than 0.");
    }
    
    int i = 3;
    
    while(i < argc)
    {
    	bfpath new_node;
    	
    	string ip(argv[i]);
    	string port(argv[i+1]);
    	
    	new_node.source_port = localport;
    	new_node.dest_port = atoi(argv[i+1]);
    	new_node.dest = ip;
    	new_node.hop_port = new_node.dest_port;
    	new_node.hop = new_node.dest;
    	new_node.cost = atof(argv[i+2]);
    	new_node.origin_cost = new_node.cost;
    	new_node.linked = true;
    	new_node.timeout_count = 0;
    	
    	neighbors[(ip + ":" + port)] = new_node;
    	RT[(ip + ":" + port)] = new_node;
    	i+=3;
    }
    
    bool up = true;
    
    string input;
    string cmd;
    string ip;
    uint16_t port;
    string key;
    
    
    map<string, bfpath> temp = neighbors;
    for(map<string, bfpath>::iterator itr=temp.begin(); itr!=temp.end(); ++itr)
	{			
		pthread_t broadcast_tr;
		if(pthread_create(&broadcast_tr, NULL, broadcast_handler, &(temp[(*itr).first])) < 0)
		{
			error("Could not create thread.");
		}
	}
    
    pthread_t listen_tr;
    if(pthread_create(&listen_tr, NULL, listening_handler, &localport) < 0)
    {
        error("Could not create thread.");
    }
    
    while(up)
    {
    	cout << ">";
    	getline(cin, input);
    	
    	cmd = input.substr(0,input.find_first_of(" "));
    	if(input.find_first_of(" ") != string::npos)
    	{
    		input = input.substr(input.find_first_of(" ") + 1);
    		ip = input.substr(0,input.find_first_of(" "));
    		input = input.substr(input.find_first_of(" ") + 1);
    		port = atoi(input.substr(0,input.find_first_of(" ")).c_str());
    		
    		struct in_addr addr;
    		if(inet_aton(ip.c_str(), &addr) == 0 || port <= 0)
    		{
    			cmd = "ERROR";
    		}
    	}
    	
    	if(cmd.compare("CLOSE") == 0)
    	{
    		up = false;
    	}
    	else if(cmd.compare("SHOWRT") == 0)
    	{
    		print_RT();
    	}
    	else if(cmd.compare("LINKDOWN") == 0)
    	{
    		key = ip + ":" + itos(port);
    		
    		neighbors[key].linked = false;
    		RT[key].linked = false;
    	}
    	else if(cmd.compare("LINKUP") == 0)
    	{
    		key = ip + ":" + itos(port);
    		
    		neighbors[key].timeout_count = 0;
    		neighbors[key].linked = true;
    		RT[key].linked = true;
    		RT[key].cost = RT[key].origin_cost;
    	}
    	else if(cmd.compare("HELP") == 0)
    	{
    		cout << endl << "SUPPORTED COMMAND LIST:" << endl;
    		cout << "SHOWRT               : Show current routing table." << endl;
    		cout << "LINKDOWN <ip> <port> : Destroy a link to a neighbor." << endl;
    		cout << "LINKUP   <ip> <port> : Restore the link destroyed by LINKDOWN." << endl;
    		cout << "CLOSE                : Close the client, acts as LINKDOWN all." << endl << endl;
    	}
    	else if(cmd.compare("TEST") == 0)
    	{
    		bf_packet packet[2];
    
			packet[0].dest_ip = 251789322;
			packet[0].dest_port = 4118;
			packet[0].cost_int = 1;
			packet[0].cost_deci = 0;
		
			packet[1].dest_ip = 251789322;
			packet[1].dest_port = 4116;
			packet[1].cost_int = 3;
			packet[1].cost_deci = 0;
		
			BF(packet, "10.0.2.15", 4117, 2);
		
			print_RT();
    	}
    	else if(cmd.compare("ERROR") == 0)
    	{
    		struct in_addr addr;
    		if(inet_aton(ip.c_str(), &addr) == 0)
    		{
    			cout << ">Invalid ip address." << endl;
    		}
    		
    		if(port <= 0)
    		{
    			cout << ">Invalid port number." << endl;
    		}
    	}
    	else
    	{
    		cout << ">Command not supported, enter HELP for list of supported command and usage." << endl;
    	}
    }

	exit(EXIT_SUCCESS);
}

void* listening_handler(void* localport)
{
	int listen;
	struct sockaddr_in incoming;
	uint16_t port = *(uint16_t*)localport;
	int len = sizeof(struct sockaddr_in);
	
	// setup listening socket
    memset(&incoming, 0, sizeof(incoming));
	incoming.sin_addr.s_addr = INADDR_ANY;
    incoming.sin_family = AF_INET;
    incoming.sin_port = htons(port);
    
    if((listen = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		shutdown(listen, SHUT_RDWR);
		error(">Failed to open UDP socket.");
	}
	
	int iSetOption = 1;
	setsockopt(listen, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));
	
	// attempt to bind the socket
	if (bind(listen, (struct sockaddr *)&incoming, sizeof(incoming)) < 0) 
	{
		shutdown(listen, SHUT_RDWR);
		error("Failed to bind UDP socket.");
	}
	
	if (setsockopt(listen, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) 
	{
		shutdown(listen, SHUT_RDWR);
		error("Failed to set timeout");
	}
	
	int timeout_count = 0;
	uint8_t buff[MAX_BUFF_SIZE];
	
	while(1)
	{
		memset(buff, 0, MAX_BUFF_SIZE);
    
		int n = recvfrom(listen, buff, MAX_BUFF_SIZE, 0, (struct sockaddr*)&incoming, (socklen_t*)&len);
		
		if(n < 0)
		{
			for(map<string, bfpath>::iterator itr=neighbors.begin(); itr!=neighbors.end(); ++itr)
			{
				string key = ((*itr).first);
				neighbors[key].timeout_count++;
				
				if(neighbors[key].timeout_count > 3)
				{
					neighbors[key].linked = false;
					
					for(map<string, bfpath>::iterator i=RT.begin(); i!=RT.end(); ++i)
					{			
						if(RT[((*i).first)].hop.compare(neighbors[key].dest) == 0)
						{
							RT[((*i).first)].linked = false;
							RT[((*i).first)].cost = INF;
						}
					}
				}
			}
		}
		else
		{
			uint8_t size = buff[0];
			uint16_t s_port;
			memcpy(&s_port, buff+1, 2);
			string s_ip = inet_ntoa(incoming.sin_addr);
			bf_packet pkt[size];
			
			memcpy(&pkt, buff + 3, sizeof(pkt));
			BF(pkt, s_ip, s_port, size);
		
			neighbors[s_ip + ":" + itos(s_port)].timeout_count = 0;
			neighbors[s_ip + ":" + itos(s_port)].linked = true;
		}
	}
}

void* broadcast_handler(void* node_info)
{
	int sd;
	struct sockaddr_in out;
	bfpath node = *(bfpath*)node_info;
	
	string d_ip = node.dest;
	uint16_t s_port = node.source_port;
	uint16_t d_port = node.dest_port;
	double cost = node.cost;
	string key = d_ip + ":" + itos(d_port);
	int len = sizeof(struct sockaddr_in);
	
	// setup listening socket
    memset(&out, 0, sizeof(out));
	out.sin_addr.s_addr =  inet_addr(d_ip.c_str());
    out.sin_family = AF_INET;
    out.sin_port = htons(d_port);
    
    if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		shutdown(sd, SHUT_RDWR);
		error(">Failed to open UDP socket.");
	}
	
	uint8_t buff[MAX_BUFF_SIZE];
	
	while(1)
	{
		if(neighbors[key].linked)
		{	
			memset(buff, 0, MAX_BUFF_SIZE);
			
			uint8_t size = RT.size();
			buff[0] = size;
			memcpy(buff + 1, &s_port, 2);
			
			bf_packet pkt[size];

			int i = 0;
			for(map<string, bfpath>::iterator itr=RT.begin(); itr!=RT.end(); ++itr)
			{			
				struct in_addr addr;
				inet_aton(RT[((*itr).first)].dest.c_str(), &addr);

				pkt[i].dest_ip = addr.s_addr;
				pkt[i].dest_port = RT[((*itr).first)].dest_port;
				pkt[i].cost_int = (int)RT[((*itr).first)].cost;
				pkt[i].cost_deci = (uint16_t)(RT[((*itr).first)].cost - pkt[i].cost_int) * COST_DIVIDER;
				
				i++;
			}
			
			memcpy(buff + 3, &pkt, sizeof(pkt));
			
			int n = sendto(sd, buff, sizeof(pkt) + 3, 0, (struct sockaddr *)&out, len);
			if(n < 0)
			{
				
				neighbors[key].linked = false;
			
				for(map<string, bfpath>::iterator itr=RT.begin(); itr!=RT.end(); ++itr)
				{			
					if(RT[((*itr).first)].hop.compare(neighbors[key].dest) == 0)
					{
						RT[((*itr).first)].linked = false;
						RT[((*itr).first)].cost = INF;
					}
				}
			}
		}
	}
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
void BF(bf_packet* route_info, string s_ip, uint16_t s_port, uint8_t count) 
{
	struct in_addr temp;
	double s_cost;
	
	string ip;
	uint16_t port;
	double cost;
	
	for(int i = 0; i < (int)count; i++)
	{
		temp.s_addr = route_info[i].dest_ip;
		
		ip = inet_ntoa(temp);
		port = route_info[i].dest_port;
		cost = route_info[i].cost_int + (double)route_info[i].cost_deci / COST_DIVIDER; 
		
		string key = ip + ":" + itos(port);
		
		if(port > 0)
		{
			if(RT.count(key) > 0)
			{
				s_cost =  RT[s_ip + ":" + itos(s_port)].cost;
				if(cost == INF)
				{
					RT[key].cost = INF;
					RT[key].linked = false;
				}
				else if(RT[key].cost > s_cost + cost)
				{
					RT[key].cost = s_cost + cost;
					RT[key].hop_port = s_port;
					RT[key].hop = s_ip;
					RT[key].linked = true;
				}
			}
			else if(myaddr.compare(key) == 0)
			{
				bfpath new_node;
						
				new_node.source_port = s_port;
				new_node.dest_port = s_port;
				new_node.dest = s_ip;
				new_node.hop_port = s_port;
				new_node.hop = s_ip;
				new_node.cost = cost;
				new_node.origin_cost = cost;
				new_node.linked = true;
			
				RT[s_ip + ":" + itos(s_port)] = new_node;
				neighbors[s_ip + ":" + itos(s_port)] = new_node;
				
				pthread_t broadcast_tr;
				if(pthread_create(&broadcast_tr, NULL, broadcast_handler, &new_node) < 0)
				{
					error("Could not create thread.");
				}
			}
			else
			{
				bfpath new_node;
				s_cost =  RT[s_ip + ":" + itos(s_port)].cost;
				
				new_node.source_port = s_port;
				new_node.dest_port = port;
				new_node.dest = ip;
				new_node.hop_port = s_port;
				new_node.hop = s_ip;
				new_node.cost = s_cost + cost;
				new_node.origin_cost = s_cost + cost;
				new_node.linked = true;
			
				RT[key] = new_node;
			}
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
/*	print_RT - print out the current routing table
/**************************************************************/
void print_RT(void)
{
	cout << endl << get_time_stamp() << " Distance vector list is:" << endl;
	
	for(map<string, bfpath>::iterator itr=RT.begin(); itr!=RT.end(); ++itr)
	{
		string key = ((*itr).first);
		bfpath val = RT[key];
		
		if(val.linked)
		{
			cout << "Destination = " << key << ", ";
			cout << "Cost = " << val.cost << ", ";
			cout << "Link = (" << val.hop << ":" << val.hop_port << ")" << endl;
		}
	}
	
	cout << endl;
}

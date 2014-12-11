/*******************************************************************
**	File name: 		Readme.txt
**	Author:			Xusheng Gao (xg2193)
**  Description:	This file describes how simple chat server work.
**
*******************************************************************/

>>>>>>>>>>>>>>>>>>>>>>> Program Description <<<<<<<<<<<<<<<<<<<<<<<<<

Simple Distributed Bellman Ford

>>>>>>>>>>>>>>>>>>>>> Development Environment <<<<<<<<<<<<<<<<<<<<<<<

Both server and client is develop under 32bit ubuntu 12.04LTS in a 
virtual machine. It is compile and build with g++ 4.6.3.

>>>>>>>>>>>>>>>>>>>>>>>>>>>>> HOW TO <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

invoke the program by:

./bfclient <local port> <ip1> <port1> <weight1> ...

Once you get in, simply type HELP for instructions

SUPPORTED COMMAND LIST:
SHOWRT               : Show current routing table.
LINKDOWN <ip> <port> : Destroy a link to a neighbor.
LINKUP   <ip> <port> : Restore the link destroyed by LINKDOWN.
CLOSE                : Close the client, acts as LINKDOWN all.

Protocol used for the packet

the packet is built for UDP, used UDP checksum for simplicity. 

Assuming small network, node count less than 256

within the packet

1st byte is packet count
next two bytes is the source port number
remaining bytes are bf_packets
	each consist of
		4 bytes for destination ip
		2 bytes for destination port
		2 bytes for integer part of the cost
		2 bytes for decimal part of the cost
		
		
ISSUES TO BE FIX:

Unable to get the packet to transmit correctly. i.e. the packet will be corrupted, 
not sure why. (could be cross thread access issue, I don't have time to do a thread 
safe inter process communication)

LINKDOWN will destroy the link.
LINKUP is unable to bring it back due to the bug in updating the routing table 
alogrithm.

SHOWRT works just fine. except the part where it will not show anything after 
the routing table failed to update

CLOSE works fine.

Due to time limit, I am unable to fix these bugs in the program.
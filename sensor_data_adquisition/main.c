#include<stdio.h> 
#include<stdlib.h> 
#include<string.h>
#include<termios.h>
#include<unistd.h> 
#include<fcntl.h>
#include"MCP39F511A.h"
#include"UART.h" 

int main(int argc, char *argv[])
{
	int fd_serie = config_serial("/dev/ttyACM0", B9600); 
	printf("Serial abierto con descriptor : %d\n", fd_serie); 

	struct read_frame *rf = create_read_frame();
	printf("\nByte array to be sent : "); 
	print_frame((unsigned char*)rf, sizeof(struct read_frame)); 

	write(fd_serie, rf, sizeof(struct read_frame)); 
	
	print_read_frame(rf); 

	/*
	// Let's suppose that we received the next byte array: 

	printf("\n");	
	unsigned char response[31]; 
	for(int i = 0; i < 31; i++) 
		response[i] = 0x01; 
	*/

	unsigned char response[31]; 
	for(int i = 0; i < 31; i++) 
		read(fd_serie, &(response[i]), 1); 

	struct response_frame *res_frame = (struct response_frame*)response;
       	printf("Byte array received : "); 	
	print_frame((unsigned char*)res_frame, sizeof(struct response_frame)); 
	
	print_readable_info_response(*res_frame); 

	close(fd_serie);
	return 0; 
}



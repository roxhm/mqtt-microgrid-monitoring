#include<stdio.h> 
#include<stdlib.h> 
#include"MCP39F511A.h"

int main()
{
	struct read_frame *rf = create_read_frame();
	
	printf("\nByte array to be sent : "); 
	print_frame((unsigned char*)rf, sizeof(struct read_frame)); 
	
	print_read_frame(rf); 

	// Let's suppose that we received the next byte array: 

	printf("\n");	
	unsigned char response[31]; 
	for(int i = 0; i < 31; i++) 
		response[i] = 0x01; 

	struct response_frame *res_frame = (struct response_frame*)response;
       	printf("Byte array received : "); 	
	print_frame((unsigned char*)res_frame, sizeof(struct response_frame)); 
	
	print_readable_info_response(*res_frame); 
	
	return 0; 
}



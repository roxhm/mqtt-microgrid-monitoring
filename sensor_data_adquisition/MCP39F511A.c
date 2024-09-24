#include<stdio.h> 
#include<stdlib.h> 
#include"MCP39F511A.h" 

unsigned char checksum(unsigned char *rf, int num_bytes) 
{
	unsigned char tmp = 0x00; 

	for(int i = 0; i < num_bytes - 1; i++) 
		tmp += rf[i]; 
	return tmp; 
}


struct read_frame* create_read_frame()
{
	struct read_frame *rf = (struct read_frame*)
				malloc(sizeof(struct read_frame)); 
       	if (rf == NULL) 
	{
		printf("For some reason, I couldn't allocate memory for creating"
		       "the read_frame. Please, find the stupid bug...\n");
		exit(0);
	}

	rf->header_byte = 0xa5; 
	rf->num_of_bytes = 0x08;	// 8 decimal 
	rf->addr.cmd_id = 0x41; 
	rf->addr.addr_high = 0x00;   
	rf->addr.addr_low = 0x02;
	rf->read.cmd_id = 0x4e; 
	rf->read.num_of_bytes = 0x1c; 	// 28 decimal
	rf->checksum = checksum((unsigned char*)rf, sizeof(struct read_frame));
	return rf; 
}


void print_read_frame(struct read_frame* rf) 
{
	printf("\n\t Byte# \t Value \t Description \n\n"); 
	printf("\t 1 \t %.2x \t Header Byte \n", 
	       	rf->header_byte);
	printf("\t 2 \t %.2x \t Number of bytes in frame \n", 
	       	rf->num_of_bytes);
	printf("\t 3 \t %.2x \t Command (Set Address Pointer) \n", 
	       	rf->addr.cmd_id);
	printf("\t 4 \t %.2x \t Address High \n", 
	       	rf->addr.addr_high);
	printf("\t 5 \t %.2x \t Address Low \n", 
	       	rf->addr.addr_low);
	printf("\t 6 \t %.2x \t Command (Register Read, n bytes) \n", 
	       	rf->read.cmd_id);
	printf("\t 7 \t %.2x \t Number of bytes to read \n", 
	       	rf->read.num_of_bytes);
	printf("\t 8 \t %.2x \t Checksum \n", 
	       	rf->checksum); 
}


void print_readable_info_response(struct response_frame rf) 
{
	int dc_mode = ((1 << 7) & rf.data_readed.system_status[1]) > 0 ? 1 : 0; 
	int sign_dccurr = ((1 << 6) & rf.data_readed.system_status[1]) > 0 ? 1 : 0; 
	int sign_dcvolt = ((1 << 5) & rf.data_readed.system_status[1]) > 0 ? 1 : 0; 
	int sign_pr = ((1 << 5) & rf.data_readed.system_status[0]) > 0 ? 1 : 0; 
	int sign_pa = ((1 << 4) & rf.data_readed.system_status[0]) > 0 ? 1 : 0; 



	printf("\n\tFrom System Status Register\n\n");

       	printf("\tDCMODE (Mode of the meter)\t\t%s\n",
		dc_mode ? "DC Mode" : "AC Mode"); 
       	printf("\tSIGN_DCCURR (Sign of DC Current RMS)\t%s\n",
		sign_dccurr ? "Positive" : "Negative"); 
       	printf("\tSIGN_DCVOLT (Sign of DC Voltage RMS)\t%s\n",
		sign_dcvolt ? "Positive" : "Negative"); 
       	printf("\tSIGN_PR (Sign of Reactive Power)\t%s\n",
		sign_pr ? "Positive, inductive and is in quadrants 1, 2" 
		: "Negative is capacitive and is in quadrants 3, 4"); 
       	printf("\tSIGN_P (Sign of Active Power)\t\t%s\n",
		sign_pa ? "Positive (import) and is in quadrants 1, 4"
		: "Negative (export) and is in quadrants 2, 3"); 

	int voltage_rms = get_value_from_byte_array(2, rf.data_readed.voltage_rms);
	int line_frequency = get_value_from_byte_array(2, rf.data_readed.line_frequency); 
	int thermistor_voltage = get_value_from_byte_array(2, rf.data_readed.thermistor_voltage); 
	int power_factor = get_value_from_byte_array(2, rf.data_readed.power_factor); 
	int current_rms = get_value_from_byte_array(4, rf.data_readed.current_rms); 
	int active_power = get_value_from_byte_array(4, rf.data_readed.active_power); 
	int reactive_power = get_value_from_byte_array(4, rf.data_readed.reactive_power); 
	int apparent_power = get_value_from_byte_array(4, rf.data_readed.apparent_power); 


	printf("\n\tVarible\t\t\tValue\n\n");
	printf("\tVoltage RMS\t\t%u\t\n", voltage_rms); 
	printf("\tLine Frequency\t\t%u\t\n", line_frequency); 
	printf("\tThermistor Voltage\t%u\t\n", thermistor_voltage);
	printf("\tPower Factor\t\t%u\t\n", power_factor);
	printf("\tCurrent RMS\t\t%u\t\n", current_rms);
	printf("\tActive Power\t\t%u\t\n", active_power);
	printf("\tReactive Power\t\t%u\t\n", reactive_power);
	printf("\tApparent Power\t\t%u\t\n", apparent_power);


}

int get_value_from_byte_array(size_t num_bytes, unsigned char* byte_array) 
{
	int pos_byte_high = num_bytes - 1; 
	int pos_byte_low = 0; 
	
	int aux = 0;
	int value = 0; 

	for(int i = pos_byte_high; i >= pos_byte_low; i--)
	{
		aux = byte_array[i]; 
		aux = aux << (8 * i);

		value += aux; 
	}

	return value; 
}

void print_frame(unsigned char* frame, size_t num_bytes) 
{
	printf("\n[ "); 
	for(int i = 0; i < num_bytes; i++) 
		printf("%.2x ", frame[i]); 
	printf("]\n"); 
}



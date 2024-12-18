#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include "MCP39F511A.h" 

byte checksum(int num_bytes, byte* frame) 
{
	byte tmp = 0x00; 

	for(int i = 0; i < num_bytes - 1; i++) 
		tmp += frame[i]; 
	return tmp; 
}


struct read_request_frame* create_read_request_frame()
{
	struct read_request_frame *rf = (struct read_request_frame*)
					malloc(sizeof(struct read_request_frame)); 
       	if (rf == NULL) 
	{
		printf("For some reason, I couldn't allocate memory for creating"
		       "the read_request_frame. Please, find the stupid bug...\n");
		exit(0);
	}

	rf->header 		= 0xa5; 
	rf->num_bytes 		= 0x08;	// 8 decimal 
	rf->addr.cmd_id 	= 0x41; 
	rf->addr.addr_high 	= 0x00;   
	rf->addr.addr_low 	= 0x02;
	rf->read.cmd_id 	= 0x4e; 
	rf->read.num_bytes 	= 0x1c; // 28 decimal
	rf->checksum = checksum(sizeof(struct read_request_frame), (byte*)rf);
	return rf; 
}


void print_read_request_frame(struct read_request_frame* rf) 
{
	printf("\n"); 

	printf("\t BYTE# \t VALUE \t DESCRIPTION \n\n"); 
	printf("\t 1 \t %.2x \t Header Byte \n", 
	       	rf->header);
	printf("\t 2 \t %.2x \t Number of bytes in frame \n", 
	       	rf->num_bytes);
	printf("\t 3 \t %.2x \t Command (Set Address Pointer) \n", 
	       	rf->addr.cmd_id);
	printf("\t 4 \t %.2x \t Address High \n", 
	       	rf->addr.addr_high);
	printf("\t 5 \t %.2x \t Address Low \n", 
	       	rf->addr.addr_low);
	printf("\t 6 \t %.2x \t Command (Register Read, n bytes) \n", 
	       	rf->read.cmd_id);
	printf("\t 7 \t %.2x \t Number of bytes to read \n", 
	       	rf->read.num_bytes);
	printf("\t 8 \t %.2x \t Checksum \n", 
	       	rf->checksum); 

	printf("\n"); 
}

// TODO: This function hasn't implemented yet. 
// 	 It has to do something similar to the
// 	 print_readable_info_response function, 
// 	 but converting all variables to float
// 	 data type. 
struct variables get_info_response(struct read_response_frame* rf)
{
	/* TODO: Qué es el thermistor_voltage? 
	 */
	float voltage_rms	= (float)get_value_from_byte_array(2, rf->data_readed.voltage_rms);
	float line_frequency 	= (float)get_value_from_byte_array(2, rf->data_readed.line_frequency); 
	float thermistor_voltage = (float)get_value_from_byte_array(2, rf->data_readed.thermistor_voltage); 
	float power_factor 	= (float)get_value_from_byte_array(2, rf->data_readed.power_factor); 
	float current_rms 	= (float)get_value_from_byte_array(4, rf->data_readed.current_rms); 
	float active_power 	= (float)get_value_from_byte_array(4, rf->data_readed.active_power); 
	float reactive_power 	= (float)get_value_from_byte_array(4, rf->data_readed.reactive_power); 
	float apparent_power 	= (float)get_value_from_byte_array(4, rf->data_readed.apparent_power); 
	
	voltage_rms = voltage_rms / 10.0; 
	line_frequency = line_frequency / 1000.0; 
	thermistor_voltage = thermistor_voltage / 10.0;
	power_factor = power_factor * 0.00003051757813; 
	current_rms = current_rms / 10000.0; 
	active_power = active_power / 100.0; 
	apparent_power = apparent_power / 100.0;
	reactive_power = reactive_power / 100.0; 

	//reactive_power = (float)sqrt(pow(apparent_power, 2) - pow(active_power, 2));

	struct variables var = 
	{
		
		.voltage_rms = voltage_rms,
		.line_frequency = line_frequency,
		.thermistor_voltage = thermistor_voltage,
		.power_factor = power_factor,
		.current_rms = current_rms,
		.active_power = active_power,
		.reactive_power = reactive_power,
		.apparent_power = apparent_power
		
		/*
		.voltage_rms = 125.6,
		.line_frequency = 60.121,  
		.thermistor_voltage = 26.613, 
		.power_factor = 0.59,
		.current_rms = 0.069,  
		.active_power = 5.40,  
		.reactive_power = 0.99,  
		.apparent_power = 9.20
		*/
	}; 
	return var; 
}

struct status get_status_from_response(struct read_response_frame* rf) 
{
	struct status var; 

	int dc_mode 	= ((1 << 7) & rf->data_readed.system_status[1]) > 0 ? 1 : 0; 
	int sign_dccurr = ((1 << 6) & rf->data_readed.system_status[1]) > 0 ? 1 : 0; 
	int sign_dcvolt = ((1 << 5) & rf->data_readed.system_status[1]) > 0 ? 1 : 0; 
	int sign_pr	= ((1 << 5) & rf->data_readed.system_status[0]) > 0 ? 1 : 0; 
	int sign_pa	= ((1 << 4) & rf->data_readed.system_status[0]) > 0 ? 1 : 0; 

	var.dc_mode = dc_mode ? "DC Mode" : "AC Mode"; 
	var.sign_dccurr = sign_dccurr ? "Positive" : "Negative";
	var.sign_dcvolt = sign_dcvolt ? "Positive" : "Negative"; 
	var.sign_pr = sign_pr ? "Positive, inductive and is in quadrants 1, 2"
			      : "Negative is capacitive and is in quadrants 3, 4"; 
	var.sign_pa = sign_pa ? "Positive (import) and is in quadrants 1, 4"
			      : "Negative (export) and is in quadrants 2, 3"; 	

	return var; 
}

void print_readable_info_response(struct read_response_frame* rf) 
{
	int dc_mode 	= ((1 << 7) & rf->data_readed.system_status[1]) > 0 ? 1 : 0; 
	int sign_dccurr = ((1 << 6) & rf->data_readed.system_status[1]) > 0 ? 1 : 0; 
	int sign_dcvolt = ((1 << 5) & rf->data_readed.system_status[1]) > 0 ? 1 : 0; 
	int sign_pr 	= ((1 << 5) & rf->data_readed.system_status[0]) > 0 ? 1 : 0; 
	int sign_pa	= ((1 << 4) & rf->data_readed.system_status[0]) > 0 ? 1 : 0; 

	printf("\n");

	printf("\tFrom System Status Register\n\n");

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

	printf("\n");
	

	int voltage_rms		= get_value_from_byte_array(2, rf->data_readed.voltage_rms);
	int line_frequency 	= get_value_from_byte_array(2, rf->data_readed.line_frequency); 
	int thermistor_voltage 	= get_value_from_byte_array(2, rf->data_readed.thermistor_voltage); 
	int power_factor 	= get_value_from_byte_array(2, rf->data_readed.power_factor); 
	int current_rms 	= get_value_from_byte_array(4, rf->data_readed.current_rms); 
	int active_power 	= get_value_from_byte_array(4, rf->data_readed.active_power); 
	int reactive_power 	= get_value_from_byte_array(4, rf->data_readed.reactive_power); 
	int apparent_power 	= get_value_from_byte_array(4, rf->data_readed.apparent_power); 

	printf("\n");

	printf("\tVARIABLE\t\t\tVALUE\n\n");
	printf("\tVoltage RMS\t\t%u\t\n", voltage_rms); 
	printf("\tLine Frequency\t\t%u\t\n", line_frequency); 
	printf("\tThermistor Voltage\t%u\t\n", thermistor_voltage);
	printf("\tPower Factor\t\t%u\t\n", power_factor);
	printf("\tCurrent RMS\t\t%u\t\n", current_rms);
	printf("\tActive Power\t\t%u\t\n", active_power);
	printf("\tReactive Power\t\t%u\t\n", reactive_power);
	printf("\tApparent Power\t\t%u\t\n", apparent_power);

	printf("\n");

}

int get_value_from_byte_array(int num_bytes, byte* array) 
{
	int pos_byte_high = num_bytes - 1; 
	int pos_byte_low = 0; 
	
	int aux = 0;
	int value = 0; 

	for(int i = pos_byte_high; i >= pos_byte_low; i--)
	{
		aux = array[i]; 
		aux = aux << (8 * i);

		value += aux; 
	}

	return value; 
}

void print_frame(int num_bytes, byte* frame) 
{
	printf("\n[ "); 
	for(int i = 0; i < num_bytes; i++) 
		printf("%.2x ", frame[i]); 
	printf("]\n"); 
}



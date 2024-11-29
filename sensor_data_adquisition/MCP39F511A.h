typedef unsigned char byte; 

/**
 * Next three structs are to represent a frame to read register bytes.  
 */
struct read_cmdpkt 
{
	byte cmd_id; 
	byte num_bytes; 
}; 

struct set_addr_ptr_cmdpkt
{
	byte cmd_id; 
	byte addr_high; 
	byte addr_low; 
};

struct read_request_frame 
{
	byte header;
	byte num_bytes;
       	struct set_addr_ptr_cmdpkt addr; 
	struct read_cmdpkt read; 
	byte checksum; 
}; 

/**
 *  Next two structs are to represent the response of the read frame sent. 
 */
struct data 
{
	byte system_status[2]; 
	byte system_version[2]; 
	byte voltage_rms[2]; 
	byte line_frequency[2]; 
	byte thermistor_voltage[2]; 
	byte power_factor[2]; 
	byte current_rms[4]; 
	byte active_power[4]; 
	byte reactive_power[4]; 
	byte apparent_power[4]; 
};

struct read_response_frame 
{
	byte response; 
	byte num_bytes; 
	struct data data_readed;  
	byte checksum; 
}; 

struct variables
{
	float voltage_rms; 
	float line_frequency; 
	float thermistor_voltage; 
	float power_factor; 
	float current_rms; 
	float active_power; 
	float reactive_power; 
	float apparent_power; 
};


/**
 * Checksum algorithm: 
 * 	Add all the bytes in the frame mod 256.
 * 	(a + b) mod 256 = (a mod 256) + (b mod 256) 
 * 
 * In this case, overflowing the variable actually gets the mod 256. 
 *
 */
byte checksum(int num_bytes, byte* frame);

struct read_request_frame* create_read_request_frame();

// TODO Please implement this fucking function. 
// I'm sick of not to have the varibales ready to be sent.
struct variables get_info_response(struct read_response_frame* rf);


/**
 * Util functions. 
 */
void print_readable_info_response(struct read_response_frame* rf);

void print_read_request_frame(struct read_request_frame* rf);
int get_value_from_byte_array(int num_bytes, byte* array);
void print_frame(int num_bytes, byte* frame); 

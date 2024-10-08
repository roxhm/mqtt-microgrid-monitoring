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

struct read_frame 
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

struct response_frame 
{
	byte ack; 
	byte num_bytes; 
	struct data data_readed;  
	byte checksum; 
}; 


/**
 * Checksum algorithm: 
 * 	Add all the bytes in the frame mod 256.
 * 	(a + b) mod 256 = (a mod 256) + (b mod 256) 
 * 
 * In this case, overflowing the variable actually gets the mod 256. 
 *
 */
byte checksum(byte *rf, int num_bytes);

struct read_frame* create_read_frame();
void print_read_frame(struct read_frame* rf);

void print_readable_info_response(struct response_frame rf);

/**
 * Util functions. 
 */
int get_value_from_byte_array(size_t num_bytes, byte* byte_array);
void print_frame(byte* frame, size_t num_bytes); 

/**
 * Next three structs are to represent a frame to read register bytes.  
 */
struct read_cmdpkt 
{
	unsigned char cmd_id; 
	unsigned char num_of_bytes; 
}; 

struct set_address_pointer_cmdpkt
{
	unsigned char cmd_id; 
	unsigned char addr_high; 
	unsigned char addr_low; 
};

struct read_frame 
{
	unsigned char header_byte;
	unsigned char num_of_bytes;
       	struct set_address_pointer_cmdpkt addr; 
	struct read_cmdpkt read; 
	unsigned char checksum; 
}; 

/**
 *  Next two structs are to represent the response of the read frame sent. 
 */
struct data 
{
	unsigned char system_status[2]; 
	unsigned char system_version[2]; 
	unsigned char line_frequency[2]; 
	unsigned char thermistor_voltage[2]; 
	unsigned char power_factor[2]; 
	unsigned char current_rms[4]; 
	unsigned char active_power[4]; 
	unsigned char reactive_power[4]; 
	unsigned char apparent_power[4]; 
};

struct response_frame 
{
	unsigned char ack; 
	unsigned char num_bytes; 
	struct data data_readed;  
	unsigned char checksum; 
}; 


/**
 * Checksum algorithm: 
 * 	Add all the bytes in the frame mod 256.
 * 	(a + b) mod 256 = (a mod 256) + (b mod 256) 
 * 
 * In this case, overflowing the variable actually gets the mod 256. 
 *
 */
unsigned char checksum(unsigned char *rf);

struct read_frame* create_read_frame();
void print_read_frame(struct read_frame* rf);

void print_readable_info_response(struct response_frame rf);

/**
 * Util functions. 
 */
int get_value_from_byte_array(size_t num_bytes, unsigned char* byte_array);
void print_frame(unsigned char* frame, size_t num_bytes); 

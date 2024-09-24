#include<stdio.h> 
#include<stdlib.h> 
#include<termios.h> 
#include<unistd.h> 
#include<fcntl.h> 

int config_serial(char *dispositivo_serial, speed_t baudios) 
{
	struct termios newtermios; 
	int fd; 

	fd = open(dispositivo_serial, (O_RDWR | O_NOCTTY) & ~O_NONBLOCK); 
	if (fd == -1) 
	{
		printf("Error al abrir el dispositivo tty \n"); 
		exit(EXIT_FAILURE); 
	}

	newtermios.c_cflag = CBAUD | CS8 | CLOCAL | CREAD; 
	newtermios.c_iflag = IGNPAR; 
	newtermios.c_oflag = 0; 
	newtermios.c_lflag = TCIOFLUSH | ~ICANON;
	newtermios.c_cc[VMIN] = 1; 
	newtermios.c_cc[VTIME] = 0;

	if (cfsetospeed(&newtermios, baudios) == -1) 
	{
		printf("Error al establecer velocidad de salida\n"); 
		exit(EXIT_FAILURE); 
	}

	if (cfsetispeed(&newtermios, baudios) == -1) 
	{
		printf("Error al establecer velocidad de entrada\n"); 
		exit(EXIT_FAILURE); 
	}

	if (tcflush(fd, TCOFLUSH) == -1) 
	{
		printf("Error al limpiar el buffer de salida\n"); 
		exit(EXIT_FAILURE); 
	}

	if (tcsetattr(fd, TCSANOW, &newtermios) == -1) 
	{
		printf("Error al establecer los parametros de la terminal\n"); 
		exit(EXIT_FAILURE); 
	}

	return fd; 
}

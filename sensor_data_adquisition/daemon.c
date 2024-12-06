#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>

void ini_daemon()
{
	pid_t pid = fork(); 

	if(pid == -1) 
	{
		perror("Error al crear el primer proceso hijo.\n");
		exit(EXIT_FAILURE);
	}

	if(pid)
	{
		printf("Se termina el proceso padre. PID del proceso hijo %d.\n", pid); 
		exit(0); 
	}

	umask(0); 

	pid_t sid = setsid(); 
	if(sid < 0) 
	{
		perror("Error al iniciar sesion.\n");
		exit(EXIT_FAILURE);
	}

	pid = fork(); 
	if(pid == -1)
	{
		perror("Error al crear el segundo proceso hijo.\n");
		exit(EXIT_FAILURE);
	}

	if(pid) 
	{
		printf("PID del segundo proceso hijo %d\n", pid);

		//FILE* fp = fopen("/var/run/demonio.pid", "w");
		FILE* fp = fopen("/home/pi/mqtt-microgrid-monitoring/sensor_data_adquisition/sensor-mqtt-daemond.pid", "w");
		if(fp)
		{
			fprintf(fp, "%d\n", pid);
			fclose(fp);
		}	
		exit(0);
	}

	chdir("/");
	
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h> 
#include <fcntl.h>
#include <syslog.h> 
#include <signal.h> 
#include "MCP39F511A.h"
#include "UART.h" 
#include "MQTTClient.h"
#include "daemon.h"

#define URL_BROKER  "ssl://3.12.36.157:8883"
#define ID_CLIENTE  "MóduloSensor"
#define QOS         1
#define TIMEOUT     1000L

#define USUARIO "UsuarioPublicador"
#define CONTRASENIA "UsuarioPublicador#1"

void envia_datos(int sig);

int main(int argc, char* argv[])
{
	ini_daemon(); 
	
	openlog("sensor-mqtt-daemon", LOG_NDELAY | LOG_PID, LOG_LOCAL0);
	
	syslog(LOG_INFO, "sensor-mqtt-daemond inicializado"); 
	
	if(signal(SIGALRM, envia_datos) == SIG_ERR) 
	{
		syslog(LOG_INFO, "Error configurando la señal SIGALRM.");
		exit(EXIT_FAILURE);
	}

	alarm(2); 
	
	while(1) 
	{
		pause();
		alarm(60);
	}

	closelog();
	return 0; 
}

void envia_datos(int sig)
{
	if(sig == SIGALRM)
	{	
		int fd_serie = config_serial("/dev/ttyACM0", B9600); 

		struct read_request_frame *request = create_read_request_frame();

		write(fd_serie, request, sizeof(struct read_request_frame)); 

		byte aux[31]; 
		for(int i = 0; i < 31; i++) 
		{	
			read(fd_serie, &(aux[i]), 1);
			usleep(5000);
		}

		struct read_response_frame *response = (struct read_response_frame*)aux;

		struct variables var = get_info_response(response);
			
		MQTTClient cliente;
		MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

		MQTTClient_deliveryToken token;
		MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
		ssl_opts.trustStore = "/home/pi/mqtt-microgrid-monitoring/sensor_data_adquisition/autoridad-certificadora.crt";
		ssl_opts.enableServerCertAuth = 1;
		conn_opts.ssl = &ssl_opts;
		conn_opts.username = USUARIO;
		conn_opts.password = CONTRASENIA;
		
		int rc;

		if ((rc = MQTTClient_create(&cliente, URL_BROKER, ID_CLIENTE,
		MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
		{
			syslog(LOG_INFO, "Error al crear el cliente. Codigo de retorno: %d\n", rc); 
			exit(0);
		}


		if ((rc = MQTTClient_connect(cliente, &conn_opts)) != MQTTCLIENT_SUCCESS)
		{
			syslog(LOG_INFO, "Error al conectar. Codigo de retorno: %d\n", rc);
			exit(0);
		}

		struct status var_status = get_status_from_response(response);

		char* status_names[5] = 
		{
			"ModeOfTheMeter",
			"SignOfDCCurrentRMS",
			"SignOfDCVoltageRMS",
			"SignOfReactivePower",
			"SignOfActivePower"
		}; 
		char* status_values[5] = 
		{
			var_status.dc_mode, 
			var_status.sign_dccurr, 
			var_status.sign_dcvolt, 
			var_status.sign_pr,
			var_status.sign_pa
		}; 
		
		for(int i = 0; i < 5; i++) 
		{
			MQTTClient_message pubmsg = MQTTClient_message_initializer;
			pubmsg.payload = status_values[i];
			pubmsg.payloadlen = (int)strlen(status_values[i]);
			pubmsg.qos = QOS;
			pubmsg.retained = 0;


			char topic[50] = "";
			char* microrred_name = "Microrred1";
			strcat(topic, microrred_name); 
			strcat(topic, "/");
			strcat(topic, status_names[i]);	

			if ((rc = MQTTClient_publishMessage(cliente, topic, &pubmsg, &token))
			!= MQTTCLIENT_SUCCESS)
			{
				syslog(LOG_INFO, "Error al publicar el mensaje. Codigo de retorno: %d\n", rc);
				exit(0);
			}

			syslog(LOG_INFO, "(%s) %s =  %s\n", ID_CLIENTE, topic, status_values[i]);
			rc = MQTTClient_waitForCompletion(cliente, token, TIMEOUT);
			syslog(LOG_INFO, "Mensaje con el token %d entregado\n", token);
		}


		char* topic_names[8] =
		{
			"VoltajeRMS",
			"LineaDeFrecuencia", 
			"VoltajeDelTermistor", 
			"FactorDePotencia", 
			"CorrienteRMS",
			"PotenciaActiva", 
			"PotenciaReactiva",
			"PotenciaAparente"
		}; 

		float datos[8] = 
		{
			var.voltage_rms, 
			var.line_frequency,
			var.thermistor_voltage,
			var.power_factor,
			var.current_rms,
			var.active_power,
			var.reactive_power,
			var.apparent_power
		}; 


		for(int i = 0; i < 8; i++) 
		{
			char variable_string[8];
			snprintf(variable_string, 8, "%f", datos[i]);

			MQTTClient_message pubmsg = MQTTClient_message_initializer;
			pubmsg.payload = variable_string;
			pubmsg.payloadlen = (int)strlen(variable_string);
			pubmsg.qos = QOS;
			pubmsg.retained = 0;


			char topic[50] = "";
			char* microrred_name = "Microrred1";
			strcat(topic, microrred_name); 
			strcat(topic, "/");
			strcat(topic, topic_names[i]);	

			if ((rc = MQTTClient_publishMessage(cliente, topic, &pubmsg, &token))
			!= MQTTCLIENT_SUCCESS)
			{
				syslog(LOG_INFO, "Error al publicar el mensaje. Codigo de retorno: %d\n", rc);
				exit(0);
			}

			syslog(LOG_INFO, "(%s) %s =  %s\n", ID_CLIENTE, topic, variable_string);
			rc = MQTTClient_waitForCompletion(cliente, token, TIMEOUT);
			syslog(LOG_INFO, "Mensaje con el token %d entregado\n", token);
		}

		if ((rc = MQTTClient_disconnect(cliente, 10000)) != MQTTCLIENT_SUCCESS)
			syslog(LOG_INFO, "Error al desconectar. Codigo de retorno: %d\n", rc);
		
		MQTTClient_destroy(&cliente);
		close(fd_serie);
	}
		//return rc;
}



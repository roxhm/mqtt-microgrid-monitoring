#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h> 
#include <fcntl.h>
#include "MCP39F511A.h"
#include "UART.h" 
#include "MQTTClient.h"
#include "daemon.h"
#include <syslog.h> 

#define URL_BROKER  "ssl://4880f67b47d8451fbfb02cc4f90ecb54.s1.eu.hivemq.cloud:8883"
#define ID_CLIENTE  "ClienteMQTTPublicador01"
#define QOS         1
#define TIMEOUT     1000L

#define USUARIO "UsuarioPublicador"
#define CONTRASENIA "UsuarioPublicador#1"

int main(int argc, char* argv[])
{
	ini_daemon(); 
	
	openlog("sensor-mqtt-demonio", LOG_NDELAY | LOG_PID, LOG_LOCAL0);
	
	syslog(LOG_INFO, "sensor-mqtt-demonio inicializado"); 

	
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

		syslog(LOG_INFO, "Esperando %d segundo para publicar el mensaje %s en el tema %s por el cliente con el identificador %s\n", (int)(TIMEOUT/1000), variable_string, topic, ID_CLIENTE);
		rc = MQTTClient_waitForCompletion(cliente, token, TIMEOUT);
		syslog(LOG_INFO, "Mensaje con el token %d entregado\n", token);
	}

	if ((rc = MQTTClient_disconnect(cliente, 10000)) != MQTTCLIENT_SUCCESS)
		syslog(LOG_INFO, "Error al desconectar. Codigo de retorno: %d\n", rc);

	MQTTClient_destroy(&cliente);
	   
	closelog();
	close(fd_serie);
       	return rc;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "SerialClass/SerialClass.h"
#define MAX_BUFFER 200
#define PAUSA_MS 200
#define N 20

typedef struct {
	char nombre[N], apellido[N];
	int edad;
	float time;
}sujeto;

void alta_sujetos(sujeto* a, int num);
void crear_fichero(sujeto* a, int num);
float verifica_sensores(Serial* Arduino, char* port);
int Enviar_y_Recibir(Serial* Arduino, const char* mensaje_enviar, char* mensaje_recibir);
float float_from_cadena(char cadena[]);
float lanzar_test(Serial* Arduino);
int Recibir(Serial* Arduino, char* mensaje_recibir);


int main(void) {
	Serial* Arduino;
	sujeto* v;
	char puerto[] = "COM3";
	int i = 0, n, edad, opcion, hay_valores = 0, posible = 0;
	char nombre[N], apellido[N], csalida, intro;
	Arduino = new Serial((char*)puerto);
	printf("¿Cuantos sujetos van a realizar la prueba?\n");
	scanf_s("%d", &n);
	v = (sujeto*)malloc(sizeof(sujeto) * n);
	if (v == NULL)
		printf("No hay memoria disponible\n");
	else {
		do
		{
			printf("\nTEST DE REACCION");
			printf("\n========================");
			printf("\n1 - Tomar datos personales.");
			printf("\n2 - Iniciar test");
			printf("\n3 - Crear registro");
			printf("\n4 - Salir");
			printf("\nIntroduzca opcion:");
			scanf_s("%d", &opcion);

			if (hay_valores == 0 && opcion > 1 && opcion < 6)
				printf("\nAntes de hacer operaciones es necesario -> 1- Tomar datos personales\n");
			else
				switch (opcion)
				{
				case 1:
					hay_valores = 1;
					alta_sujetos(v, n);
					break;
				case 2:
					if (i < n) {
						v[i].time = verifica_sensores(Arduino, puerto);
						if (v[i].time != 0) {
							i++;
							posible = 1;
						}
					}
					else {
						printf("\nNo hay memoria para mas test\n");
					}
					break;
				case 3:
					if (posible == 0) {
						printf("\nNo es posible crear el fichero sin realizar antes una prueba\n");
					}
					else {
						crear_fichero(v, n);
					}
					break;
				case 4:
					break;
				default:
					printf("\nOpcion inexistente\n");
				}
		} while (opcion != 4);
	}
	return 0;
}

void alta_sujetos(sujeto* a, int num)
{
	int i;
	for (i = 0; i < num; i++){
		printf("Dime tu nombre: \n");
		fgets(a[i].nombre, N, stdin);
		printf("Dime tu apellido: \n");
		fgets(a[i].apellido, N, stdin);
		printf("Dime tu edad: \n");
		scanf_s("%d", &a[i].edad);
	}
}

void crear_fichero(sujeto* a, int num)
{
	FILE* fichero;
	int i;
	errno_t e;
	e = fopen_s(&fichero, "Listado_Sujetos.txt", "wt");
	if (fichero == NULL)
		printf("No se ha podido guardar los datos\n");
	else
	{
		for (i = 0; i < num; i++)
		{
			fprintf(fichero, "%s\n", a[i].nombre);
			fprintf(fichero, "%s\n", a[i].apellido);
			fprintf(fichero, "%d\n", a[i].edad);
			fprintf(fichero, "%.3f\n", a[i].time);
			fprintf(fichero, "===========================\n");
		}
		fclose(fichero);
	}

}

float verifica_sensores(Serial* Arduino, char* port)
{
	float tiempo = 0;
	if (Arduino->IsConnected())
	{
		start(Arduino);
		tiempo = lanzar_test(Arduino);
		if (tiempo != -1)
			printf("\nTiempo: %f\n", tiempo);
	}
	else
	{
		printf("\nNo se ha podido conectar con Arduino.\n");
		printf("Revise la conexión, el puerto %s y desactive el monitor serie del IDE de Arduino.\n",port);
	}
	return tiempo;
}

int Enviar_y_Recibir(Serial* Arduino, const char* mensaje_enviar, char* mensaje_recibir)
{
	int bytes_recibidos = 0, total = 0;
	int intentos = 0, fin_linea = 0;
	Arduino->WriteData((char*)mensaje_enviar, strlen(mensaje_enviar));
	Sleep(PAUSA_MS);
	bytes_recibidos = Arduino->ReadData(mensaje_recibir, sizeof(char) * MAX_BUFFER - 1);
	while ((bytes_recibidos > 0 || intentos < 5) && fin_linea == 0)
	{
		if (bytes_recibidos > 0)
		{
			total += bytes_recibidos;
			if (mensaje_recibir[total - 1] == 13 || mensaje_recibir[total - 1] == 10)
				fin_linea = 1;
		}
		else
			intentos++;
		Sleep(PAUSA_MS);
		bytes_recibidos = Arduino->ReadData(mensaje_recibir + total, sizeof(char) * MAX_BUFFER - 1);
	}
	if (total > 0)
		mensaje_recibir[total - 1] = '\0';
	return total;
}

float float_from_cadena(char cadena[]) {
	int i, estado = 0, divisor = 10;
	float numero;
	for (i = 0; i < MAX_BUFFER || cadena[i] != '\0'; i++) {
		switch (estado){
		case 0:
			if (cadena[i] >= '0' && cadena[i] <= '9')
			{
				numero = cadena[i] - '0';
				estado = 1;
			}
			break;
		case 1:
			if (cadena[i] >= '0' && cadena[i] <= '9')
				numero = numero * 10 + cadena[i] - '0';
			else
				if (cadena[i] == '.' || cadena[i] == ',')
					estado = 2;
				else
					estado = 3;
			break;
		case 2:
			if (cadena[i] >= '0' && cadena[i] <= '9')
			{
				numero = numero + (float)(cadena[i] - '0') / divisor;
				divisor *= 10;
			}
			else
				estado = 3;
			break;
		}
	}
	return numero;
}

float lanzar_test(Serial* Arduino)
{

	float tiempo;
	int bytesRecibidos;
	char mensaje_recibido[MAX_BUFFER];

	if (Arduino->IsConnected())
	{
		bytesRecibidos = Enviar_y_Recibir(Arduino, "START_TEST\n", mensaje_recibido);
		if (bytesRecibidos > 0)
		{
			bytesRecibidos = Enviar_y_Recibir(Arduino, "GET_TIEMPO\n", mensaje_recibido);
			if (bytesRecibidos <= 0)
			{
				printf("\nNo se ha recibido respuesta a la petición\n");
				tiempo = -1;
			}
			else
			{
				printf("\nLa respuesta recibida tiene %d bytes. Recibido=%s\n", bytesRecibidos, mensaje_recibido);
				bytesRecibidos = Recibir(Arduino, mensaje_recibido);
				if (bytesRecibidos > 0)
					tiempo = float_from_cadena(mensaje_recibido);
				else
				{
					printf("Timeout sin recibir respuesta del test\n");
					tiempo = -1;
				}
			}
			return tiempo;
		}else{
		printf("\nNo se ha recibido respuesta a la petición\n");
				tiempo = -1;
				return tiempo;
		}
	}
}

int Recibir(Serial* Arduino, char* mensaje_recibir)
{
	int bytes_recibidos = 0, total = 0;
	int intentos = 0, fin_linea = 0;
	bytes_recibidos = Arduino->ReadData(mensaje_recibir, sizeof(char) * MAX_BUFFER - 1);

	while ((bytes_recibidos > 0 || intentos < 10) && fin_linea == 0)
	{
		if (bytes_recibidos > 0)
		{
			total += bytes_recibidos;
			if (mensaje_recibir[total - 1] == 13 || mensaje_recibir[total - 1] == 10)
				fin_linea = 1;
		}
		else
			intentos++;
		Sleep(2 * PAUSA_MS);
		bytes_recibidos = Arduino->ReadData(mensaje_recibir + total, sizeof(char) * MAX_BUFFER - 1);
	}
	if (total > 0)
		mensaje_recibir[total - 1] = '\0';
	return total;
}

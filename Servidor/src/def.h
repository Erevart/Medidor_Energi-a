/*
    Lectura Contador de Energia

*/

/* ============================================================================================================ */
/* ============================================================================================================ */
/*                                        PARAMETROS DE CONFIGURACION
/* ============================================================================================================ */
/* ============================================================================================================ */

//#define MCP39F501     // Comunicacion con Contador de Energia MCP39F501

/* ============================================================================================================ */
/* ============================================================================================================ */
/* ============================================================================================================ */

/* -------------------------------------------------------------------------------------------------------------*/

/* Parametros de Debug */
#define _DEBUG_WIFI   // Muestra por puerto serie información relativa la configuracion wifi.
#define _DEBUG_COMUNICACION
#define _DEBUG_COMUNICACION_LIMIT
//#define _DEBUG_ERROR  // Muestra los mensajes de error.
//#define _DEBUG_TX     // Muestra la información que transmitiría al MCP.
//#define _DEBUG_RX     // Muestra la información que recibiría del MCP



/* Definicion de Parametros */
#define uart Serial     // Esta definición permite modificar el modulo de comunicación utilizado para
                        // comunicarse con el Contador de Energia.
#define debug Serial    // Esta definición permite modificar el modulo de comunicación utilizado para
                        // mostrar los mensajes de debug.

/* Macros */

/*********************************/
/* Definicion Puertos (ESP82666) */
/********************************/

#define GPIO_SINC  0   // GPIO 0
#define GPIO3  3 // GPIO 3
#define GPIO4  4 // GPIO 4
#define GPIO5  5 // GPIO 5
#define GPIO6  6 // GPIO 6

/***************
/* Constantes */
/**************/

/* Frecuencias de Procesos */
#define MAX_PERIODO     10                                // Periodo tiempo para realizar las lecturas del chips (ms).
// Solo es necesario indicar el periodo de tiempo en segundo con el que debe repetirse la tarea.
// El tiempo se indica aqui ! // (ms* (MAX_PERIODO / 1000000))
#define loop1       2     // Loop de 20 ms
#define loop2       4     // Loop de 40 ms
#define loop3       100   // Loop de 1 s
#define loop4       300   // Loop de 3 s

/* Parametros Wifi */
#define PRE_SSID "MCPESP_"
#define CONTRASENA "zxcvmnbv1234"
#define TIEMPO_RESET 30000 // Tiempo de espera para realizar borrar los datos de configuración de la red wifi.
#define MCPESP_SERVER_PORT 23

/* Parametros Comunicacion ESP8266 - ESP8266 */
#define USUARIO_REGISTRADO 0xEE
#define WACK 0xCC
#define MAX_INTENTOS 1           // Numero de intentos para establecer la comunicacion sino se ha tansmitido correctamente.
#define MCPESP_SERVER_PORT 23      // Puerto de conexión a los servidores.
#define TCP_TIEMPO_CONEXION 120  // Tiempo de esperar, para cerrar la comunicación TCP, después de parar la comunicación.
#define MAX_ESPWIFI 15000



/* Prototipo de Funciones */
void servidor_tcp();


/* Declaración de funciones */

void configWifi();
void check_clientes();
void comunicacion_cliente();
void isrsinc();


/* Variables */
uint32_t temp = 0;
uint32_t tension = 0;
uint32_t corriente = 0;
uint32_t frecuencia = 0;
char CMD = '$';
bool reset_wifi = false;
os_timer_t *timerrest;


unsigned long currentMillis = 0;    // Variable que indica el tiempo actual.
unsigned long previousMillis = 0;   // Variable que indica el instante de tiempo en el que se realizo la ultima ejecucion del bucle principal.
unsigned long loop2_previousTime = 0; // Variable que indica el instante de tiempo en el que se ejecuto la loop2_
uint32_t timecounter = 0;           // Variable que indica el numero de iteracciones del bucle principal.
struct espconn* esp_conn;
bool registrado = false;
bool tcp_establecido = false;
bool tcp_desconectado = false;
bool tcp_finalizar = false;
bool transmision_finalizada = true;

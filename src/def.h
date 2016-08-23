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
#define _DEBUG_WIFI         // Muestra por puerto serie información relativa la configuración wifi.
#define _DEBUG_COMUNICACION // Muestra por puerto serie información relativa la comunicación TCP.
//#define _DEBUG_ERROR      // Muestra los mensajes de error.
//#define _DEBUG_TX         // Muestra la información que transmitiría al MCP.
//#define _DEBUG_RX         // Muestra la información que recibiría del MCP



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
#define PRE_SSID "MCPESP_"          // Prefijo de la red a buscar.
#define CONTRASENA "zxcvmnbv1234"   // Contraseña
#define TIEMPO_RESET 10000          // Tiempo de espera para realizar borrar los datos de configuración de la red wifi.
#define MCPESP_SERVER_PORT 23       // Puerto de conexión a los servidores.

/* Parametros Comunicacion ESP8266 - ESP8266 */
#define TCP_TIEMPO_CONEXION 120     // Tiempo de inactividad, para cerrar la comunicación TCP.
#define MAX_ESPWIFI 15000           // Tiempo de espera antes de considerar que la comunicacion wifi ha sido perdida.
#define USUARIO_REGISTRADO 0xEE
#define WACK 0xCC

/* Parametros comunicacion ESP8266 - MCP39F51X */
#define MAX_INTENTOS 1           // Numero de intentos para establecer la comunicacion sino se ha tansmitido correctamente.



/* Prototipo de Funciones */

// Confwifi
void servidor_tcp();
void configWifi();
void comunicacion_cliente();
void isrsinc();


/* Variables */
struct espconn* esp_conn;             // Estructura que identifica y determina la comunicación TCP.

os_timer_t *timerrest;      // Variable timer por software

unsigned long currentMillis = 0;      // Variable que indica el tiempo actual.
unsigned long previousMillis = 0;     // Variable que indica el instante de tiempo en el que se realizo la ultima ejecucion del bucle principal.
unsigned long loop2_previousTime = 0; // Variable que indica el instante de tiempo en el que se ejecuto la loop2_

/* Aún en pruebas */
uint32_t temp = 0;
uint32_t tension = 0;
uint32_t corriente = 0;
uint32_t frecuencia = 0;
/* Aún en pruebas */
uint32_t timecounter = 0;             // Variable que indica el numero de iteracciones del bucle principal.

int8_t estadoscan = false;

char CMD = '$';             // Identifica la operación solicitda. El simbolo $ indica el estado de reposo.


bool registrado = false;              // Indica si el dispositivo ha sido registrado en la red wifi conectada.
bool tcp_establecido = false;         // TCP establecido.
bool tcp_desconectado = false;        // TCP desconectado.
bool transmision_finalizada = true;   // Transmision de datos por TCP realizada.
bool reset_wifi = false;              // Determina si la red wifi registrada en memoria es diferente a la escenada y por lo tanto
                                      // es necesario confirmar el registro del dispositivo. Se supone que el dispositivo no procede
                                      // no procede de un reset y por lo tanto no es necersario al confirmación de registro.

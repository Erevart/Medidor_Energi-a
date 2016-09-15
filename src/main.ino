
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <inttypes.h>

extern "C" {
  #include <user_interface.h>
  #include <espconn.h>
  #include <mem.h>
  #include <ets_sys.h>
  #include <osapi.h>
}

#include <def.h>
#include <rtcTime.h>
#include <GPIO.h>
#include <MCP39F511X.h>
#include <comtcp.h>
#include <confwifi.h>

void setup() {

  /*****************/
  /* RTC timer     */
  /*****************/
  update_rtc_time(false);

  /**********************************/
  /*   Definicion Puerto I/O        */
  /**********************************/
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(2, OUTPUT);

  /******************************/
  /*   Configuración UART       */
  /******************************/
  Serial.begin(115200);

  /******************************/
  /* Comprobacion Comunicacion  */
  /******************************/

  // Se verifica antes de comenzar la ejecucion
  // del programa que el chips se encuentra
  // conectado y listo para comunicarse
  /*  if (!isReadyMCP39F511N()){
      digitalWrite(2,HIGH);
      delay(2000);
      system_restart();
    }
  */  
  /*****************/
  /* Interrupción  */
  /*****************/
  attachInterrupt(GPIO_SINC,isrsinc,FALLING);

  /******************************/
  /*   Configuración Wifi       */
  /******************************/
  configWifi();

  // En el supuesto de conectarse a una red ya identificada no se exige
  // la confirmación de conexion.
  if (reset_wifi)
    // Se comprueba que el usuario ha sido registrado en la red.
    // en caso contrario se borran los parametros de conexión.
    if (!check_connection()){
      reset_configwifi(NULL);
    }


  /******************************/
  /*  Servidor TCP      */
  /******************************/
  servidor_tcp();

  delay(2000);
}

void loop() {

 // Se actualiza la variable tiempo, para conocer el tiempo transcurrido
 currentMillis = millis();

 if (currentMillis - previousMillis >= MAX_PERIODO) {

   // Se guarda en el último instante de tiempo en el,
   // que se ejecuta el contenido del bucle principal.
   previousMillis = currentMillis;

   /**************************************
     Frecuencia de Refresco: 50 Hz
   *************************************/
   if (timecounter % loop1 == 0){
     // Si un cliente ha establecido una comunicación se envian los datos solicitados.
     if (tcp_establecido)
      comunicacion_cliente();

  }

   /**************************************
     Frecuencia de Refresco: 25 Hz
   *************************************/
   if (timecounter % loop2 == 0){
     digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    // MCPwrite(0x007A,0x04,0x00002000);
    //temp = MCPread(0x007E,0x04);
    /*
    if (temp == 0x00400000){
    //  MCPwrite(0x007E,0x04,0);
      digitalWrite(2,HIGH);
    }
    else{
    //  MCPwrite(0x007E,0x04, 0x00400000);//0x00400000
      digitalWrite(2,LOW);
    }
    */

   }
   else if ( (currentMillis - loop2_previousTime) >= 40){
     // Se guarda en el último instante de tiempo en el,
     // que se ejecuta el contenido del loop2.
     loop2_previousTime = currentMillis;

    //tension = MCPread(ADDR[VOLTAGE_RMS],TYPE_DATA[VOLTAGE_RMS]);
    //corriente = 0.0001 * MCPread(ADDR[CURRENT_RMS],TYPE_DATA[CURRENT_RMS]);
    //frecuencia = MCPread(ADDR[LINE_FREQUENCY],TYPE_DATA[LINE_FREQUENCY]);

   }

   /**************************************
     Frecuencia de Refresco:  1 Hz
   *************************************/
   if (timecounter % loop3 == 0){

     get_rtc_time();

   }

   /**************************************
     Frecuencia de Refresco:  1/4 Hz
   *************************************/
   if (timecounter % loop4 == 0){
     // Si el cliente se desconecta, se cierra el canal de comunicación y
     // se abre una nuevo canal de comunicación para futuras conexiones.
     if (tcp_desconectado){
      #ifdef _DEBUG_COMUNICACION_CONEXION
        int8_t info_espconn = espconn_delete(esp_conn);
        debug.println("[MAIN] Cierre del canal de comunicacion");
        debug.println(info_espconn);
      #else
        espconn_delete(esp_conn);
      #endif
      servidor_tcp();
     }
   }

   // La variable timecounter debe reiniciarse cuando se alcance
   // el numero de ciclos de las acciones de menor prioridad.
   if (timecounter == 1200)
     timecounter = 0;
   else
     timecounter++;

 }
 yield();

}

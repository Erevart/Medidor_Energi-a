/*

 conmtcp.c - Funciones de comunicacion para la comunicación TCP entre dispositivos ESP8266 - ESP8266.

 */


/******************************************************************************
 * Función : tcp_sent
 * @brief  : Envia al cliente conectado por comunicación TCP los datos indicados.
 * @param  : pdata - Puntero al array de datos que se desea enviar
 * @return : true - El envio de los datos se ha realizado correctamente.
 * @return : false - El envio de los datos no se ha podido realizar.
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "TCP_ST".
 *******************************************************************************/
bool tcp_sent(uint8_t *pdata){

  unsigned long time0;
  int8_t info_tcp = -1;

  transmision_finalizada = false;
  time0 = millis();

  while (!transmision_finalizada){

    yield();

    if (info_tcp != ESPCONN_OK)
       info_tcp = espconn_send(esp_conn, pdata , sizeof(pdata)/sizeof(uint8_t));

    #ifdef _DEBUG_COMUNICACION
      debug.print("[TCP_ST] Codigo de envio: ");
      debug.println(info_tcp);
    #endif

    if ((millis()-time0)>MAX_ESPWIFI){
      /* AÚN POR COMPROBAR */
      espconn_disconnect(esp_conn);
      tcp_desconectado = true;
      /* AÚN POR COMPROBAR */
      return false;
    }
  }

  return true;
}


 /******************************************************************************
  * Función : comunicacion_cliente
  * @brief  : Se envia la información solicitada por el cliente a través de la
              comunicación TCP establecida previamente.
  * @param  : none
  * @return : none
  * Etiqueta debug : Todos los comentarios para depuración de esta función
                    estarán asociados a la etiqueta: "COMCL".
  * ------------------------ *
   Protocolo de comunicación
  * ------------------------ *

 |------------|--------------|---------------|---------|--------|-------------------|
 | -- Start --|-- tcpcount --|-- ident_var --|-- Var --|- \...\-|-- Stop/Continue --|
 |------------|--------------|---------------|---------|--------|-------------------|

 Start (start) - uint8_t = ¿  || Byte de inicio de comunicación.
 tcpcount      - uint8_t =    || Número de variables que serán recibidas.
 ident_var     - *uint8_t =   || Identificador de la variable recibida.
 Var           - *double      || Variable
 Stop/Continue - uint8_t = #/?|| Byte de fin de comunicación o indicador de mantener la comunicación.

  -- Ejemplo de transmision --
  1º Transmision de una variable y no se finaliza comunicacion.
  char psent2[200];
  sprintf(psent2, "¿%"PRIu8"%"PRIu8"%f?",1,'!',3.141516)
  char psent2[200];
  sprintf(psent2, "¿%"PRIu8"%"PRIu8"%f#",1,'!',141516.3)


u
  *******************************************************************************/
void comunicacion_cliente(){

  uint8_t psent[200];

  union {
  float float_value;
  uint8_t byte[4];
} data;

  // Se comprueba si las operaciones solicitadas ya han sido realizadas.
  if (CMD == '$' || !transmision_finalizada)
    return;

  switch (CMD)
  {
    // Se realiza el proceso de registro en supuesto de ser solicitado.
    case USUARIO_REGISTRADO:
      #ifdef _DEBUG_COMUNICACION
      debug.print("[COMCL] Confirmacion de registro.");
      unsigned long time0;
      time0 = millis();
      #endif

      psent[0] = TCP_START;
      psent[1] = 1;
      psent[2] = WACK;
      psent[3] = TCP_STOP;

      if (tcp_sent(psent))
        registrado  = true;
      else{
        registrado = false;
        return;
      }

      // Se actualiza el contador de tiempo.
      update_rtc_time(true);

      #ifdef _DEBUG_COMUNICACION
       debug.println("-----------------------------------------------");
       debug.print("[COMCL] REGISTRADO. Tiempo requerido: ");
       debug.println(millis()-time0);
       debug.println("-----------------------------------------------");
      #endif
     break;

     default:
       #ifdef _DEBUG_COMUNICACION
         debug.print("[COMCL] Operacion no identificada. CMD: ");
         debug.println(CMD);
       #endif
     break;

     // Operación de depuración
       case '!':
        #ifdef _DEBUG_COMUNICACION
          debug.print("[COMCL] !Soy el servidor: ");
          debug.println(ESP.getChipId());
        #endif
         //char psent2[200];
         //sprintf(psent2, "¿2Soy el servidor: %d, Tiempo: %llu \r\n", ESP.getChipId(),\
         ( get_rtc_time() / 10000000) / 100);
         //char psent2[200];
/*         sprintf(psent, "¿%i%i%f?",1,'!',3.141516);

         tcp_sent(reinterpret_cast<uint8_t*>(psent));
*/
         //sprintf(psent2, "¿2Soy el servidor:");

         float p = 10.5;
         data.float_value = p;
         psent[0] = TCP_START;
         psent[1] = 1;
         psent[2] = 0x21;
         Serial.print('A');
         Serial.write(data.byte[0]);
         Serial.write(data.byte[1]);
         Serial.write(data.byte[2]);
         Serial.write(data.byte[3]);
         psent[3] = data.byte[0];
         psent[4] = data.byte[1];
         psent[5] = data.byte[2];
         psent[6] = data.byte[3];
         Serial.print('A');
         psent[7] = TCP_STOP;



        /* for (uint8_t i = 0; i < 4; i++)
            psent[i+4] = data.byte[i];
         //psent[7] = TCP_STOP;
        */
        // tcp_sent(psent);

        int8_t info_tcp = -1;

        transmision_finalizada = false;

        while (!transmision_finalizada){

          yield();

          if (info_tcp != ESPCONN_OK)
             info_tcp = espconn_send(esp_conn, psent , 8);

          #ifdef _DEBUG_COMUNICACION
            debug.print("[TCP_ST] Codigo de envio: ");
            debug.println(info_tcp);
          #endif

        }

       break;
   }

   // Se indica que la operación solicitada ya ha sido realizada.
   CMD = '$';

}

/******************************************************************************
 * Función : tcp_server_sent_cb
 * @brief  : Callback cuando la transmisión es finalizada. Determina que los
             ha sido enviados correctamente.
 * @param  : arg - puntero a la variable tipo espconn que determina la comunicación.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "TCP_ST_CB".
 *******************************************************************************/
void tcp_server_sent_cb(void *arg){
 //Datos enviados correctamente
 transmision_finalizada = true;

 #ifdef _DEBUG_COMUNICACION
   debug.println("[TCP_ST_CB] TRANSMISION REALIZADA CORRECTAMENTE");
 #endif
}

/******************************************************************************
 * Función : tcp_server_discon_cb
 * @brief  : Callback cuando la comunicación tcp se finaliza. Indica que la
             comunicación tcp ha finalizado.
 * @param  : arg - puntero a la variable tipo espconn que determina la comunicación.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "TCP_DC_CB".
 *******************************************************************************/
void tcp_server_discon_cb(void *arg){
 //tcp disconnect successfully
 tcp_desconectado = true;
 tcp_establecido = false;
 transmision_finalizada = true;


 #ifdef _DEBUG_COMUNICACION
   debug.println("[TCP_DC_CB] DESCONEXION REALIZADA CORRECTAMENTE");
 #endif
}


/******************************************************************************
 * Función : tcp_server_recon_cb
 * @brief  : Callback cuando la comunicación tcp es interrumpida. Indica que la
             comunicación tcp ha sido forzada a cerrarse.
 * @param  : arg - puntero a la variable tipo espconn que determina la comunicación.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "TCP_RC_CB".
 *******************************************************************************/
void tcp_server_recon_cb(void *arg, sint8 err){
   // Se ha producido una fallo, y la conexión ha sido cerrada.
   tcp_desconectado = true;
   tcp_establecido = false;
   transmision_finalizada = true;

   #ifdef _DEBUG_COMUNICACION
       debug.print("[TCP_RC_CB] CONEXION INTERRUMPIDA. CODIGO DE ERROR: ");
       debug.println(err);
   #endif
}


/******************************************************************************
 * Función : tcp_server_recv_cb
 * @brief  : Callback cuando se recibe información del cliente. Permite leer la
             la trama de datos recibida e identificar la operación solicitada.
 * @param  : arg - puntero a la variable tipo espconn que determina la comunicación.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "TCP_RV_CB".
 *******************************************************************************/
void tcp_server_recv_cb(void *arg, char *tcp_data, unsigned short length){
   // Indicador de recepción de datos.
   digitalWrite(2, !digitalRead(2));

   #ifdef _DEBUG_COMUNICACION
       debug.print("[TCP_RV_CB] Recepcion de datos. Información recibida: ");
       debug.println(tcp_data);
   #endif

   /* PROCESAMIENTO DE LA INFORMACIÓN RECIBIDA */
   switch (tcp_data[0]) {

       case USUARIO_REGISTRADO:
         #ifdef _DEBUG_COMUNICACION
           debug.println("[TCP_RV_CB] Proceso de registro");
         #endif
         CMD = tcp_data[0];
       break;


       default:
         #ifdef _DEBUG_COMUNICACION
           debug.println("[TCP_RV_CB]: Operacion no identificado.");
         #endif
       break;

       // Operación de debug.
         case '!':
             #ifdef _DEBUG_COMUNICACION
              debug.println("TCP: Comunicacion !");
             #endif
             CMD = tcp_data[0];
         break;
   }

}

/******************************************************************************
 * Función : tcp_listen
 * @brief  : Callback cuando se establece la comunicación TCP. Permite identificar
             cuando se ha iniciado a la comunicación y establecer las funciones
             de Callback para los distintos eventos de la comunicación TCP.
 * @param  : arg - puntero a la variable tipo espconn que determina la comunicación.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "TCPL".
 *******************************************************************************/
void tcp_listen(void *arg){
  #ifdef _DEBUG_COMUNICACION
   debug.println("[TCPL] Comunicacion iniciada");
  #endif

  struct espconn *pesp_conn = static_cast<struct espconn *> (arg);

  /* Función llamada cuando se reciben datos */
  espconn_regist_recvcb(pesp_conn, tcp_server_recv_cb);
  /* Función llamada cuando la conexión es interrumpida */
  espconn_regist_reconcb(pesp_conn, tcp_server_recon_cb);
  /* Función llamada cuando se finaliza la conexión */
  espconn_regist_disconcb(pesp_conn, tcp_server_discon_cb);
  /* Función llamada cuando los datos se han enviado correctamente */
  espconn_regist_sentcb(pesp_conn, tcp_server_sent_cb);

  tcp_establecido = true;
  tcp_desconectado = false;
  transmision_finalizada = true;

}

/******************************************************************************
* Función : servidor_tcp
* @brief  : Callback cuando se establece la comunicación TCP. Permite identificar
            cuando se ha iniciado a la comunicación y establecer las funciones
            de Callback para los distintos eventos de la comunicación TCP.
* @param  : none
* @return : none
* Etiqueta debug : Todos los comentarios para depuración de esta función
                  estarán asociados a la etiqueta: "CTCP".
*******************************************************************************/
void servidor_tcp(){

 int8_t info_tcp;
 unsigned long time0;

 #ifdef _DEBUG_COMUNICACION
   debug.println("[CTCP] Arranque del servidor tcp.");
 #endif

 // Configuración de los parámetros de la comunicación TCP
 esp_conn = (struct espconn *)os_malloc((uint32)sizeof(struct espconn));
 esp_conn->type = ESPCONN_TCP;
 esp_conn->state = ESPCONN_NONE;
 esp_conn->proto.tcp = (esp_tcp *)os_malloc((uint32)sizeof(esp_tcp));
 esp_conn->proto.tcp->local_port = MCPESP_SERVER_PORT;

 time0 = millis();
 #ifdef _DEBUG_COMUNICACION
   info_tcp = espconn_accept(esp_conn);
   debug.print("[CTCP] Codigo de arranque: ");
   debug.println(info_tcp);

   while(info_tcp != ESPCONN_OK){
     yield();
     debug.print("[CTCP] Estableciendo servidor TCP. Tiempo requerido: ");
     debug.println(millis()-time0);
     info_tcp = espconn_accept(esp_conn);
     if ((millis()-time0)>MAX_ESPWIFI){
       return;
     }
   }
 #else
   while(espconn_accept(esp_conn) != ESPCONN_OK ){
     yield();
     if ((millis()-time0)>MAX_ESPWIFI)
       return;
     }
 #endif

 // Tiempo de inactividad que debe transcurrir para finalizar la conexión.
 espconn_regist_time(esp_conn, TCP_TIEMPO_CONEXION, 1);

 // Se establace la función que será invocada cuando se inicie la comunicación.
 espconn_regist_connectcb(esp_conn, tcp_listen);

 #ifdef _DEBUG_COMUNICACION
   debug.println("[CTCP] SERVICIO TCP: Establecido.");
 #endif

}

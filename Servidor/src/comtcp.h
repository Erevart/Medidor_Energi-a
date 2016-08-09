/*

 conmtcp.c - Funciones de comunicacion para la comunicación TCP entre dispositivos ESP8266 - ESP8266.

 */


/******************************************************************************
 * Función : tcp_sent
 * @brief  : Envia al cliente conectado por comunicación TCP los datos indicados.
 * @param  : pdata - Puntero al array de datos que se desea enviar
 * @return : true - El envio de los datos se ha realizado correctamente.
 * @return : false - El envio de los datos no se ha podido realizar.
 * Etiqueta debug : Todos los comentarios para depueración de esta función
                   estarán asociados a la etiqueta: "TCP_ST".
 *******************************************************************************/
bool tcp_sent(uint8_t *pdata){

  unsigned long time0;
  int8_t info_envio = -1;

  transmision_finalizada = false;
  time0 = millis();

  while (!transmision_finalizada){

    yield();

    if (info_envio != ESPCONN_OK)
       info_envio = espconn_send(esp_conn, pdata , strlen(reinterpret_cast<char*>(pdata)));

    #ifdef _DEBUG_COMUNICACION
      debug.print("[TCP_ST] Codigo de envio: ");
      debug.println(res_envio);
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
  * Etiqueta debug : Todos los comentarios para depueración de esta función
                    estarán asociados a la etiqueta: "COMCL".
  *******************************************************************************/
void comunicacion_cliente(){

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

      uint8_t psent[1];
      psent[0] = WACK;

      if (tcp_sent(psent))
        registrado  = true;
      else{
        registrado = false;
        return;
      }

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

     // Operación de depueración
     #ifdef _DEBUG_COMUNICACION
       case '!':
         debug.println("[COMCL] !Soy el servidor 1.");
         char psent2[18];
         sprintf(psent2, "!Soy el servidor 1");
         tcp_sent(reinterpret_cast<uint8_t*>(psent2));
       break;
    #endif
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
 * Etiqueta debug : Todos los comentarios para depueración de esta función
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
 * Función : tcp_server_sent_cb
 * @brief  : Callback cuando la comunicación tcp se finaliza. Indica que la
             comunicación tcp ha finalizado.
 * @param  : arg - puntero a la variable tipo espconn que determina la comunicación.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depueración de esta función
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
 * Etiqueta debug : Todos los comentarios para depueración de esta función
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
 * Etiqueta debug : Todos los comentarios para depueración de esta función
                   estarán asociados a la etiqueta: "TCP_RV_CB".
 *******************************************************************************/
void tcp_server_recv_cb(void *arg, char *tcp_data, unsigned short length){
   // Indicador de recepción de datos.
   digitalWrite(2, !digitalRead(2));

   #ifdef _DEBUG_COMUNICACION_LIMIT
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
       #ifdef _DEBUG_COMUNICACION
         case '!':
             debug.println("TCP: Comunicacion !");
             CMD = tcp_data[0];
         break;
       #endif
   }

}

/******************************************************************************
 * Función : tcp_listen
 * @brief  : Callback cuando se establece la comunicación TCP. Permite identificar
             cuando se ha iniciado a la comunicación y establecer las funciones
             de Callback para los distintos eventos de la comunicación TCP.
 * @param  : arg - puntero a la variable tipo espconn que determina la comunicación.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depueración de esta función
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
* Etiqueta debug : Todos los comentarios para depueración de esta función
                  estarán asociados a la etiqueta: "CTCP".
*******************************************************************************/
void servidor_tcp(){

 int8_t info_con;

 #ifdef _DEBUG_COMUNICACION_CONEXION
   debug.println("[CTCP] Arranque del servidor tcp.");
   unsigned long time0 = millis();
 #endif

 // Configuración de los parámetros de la comunicación TCP
 esp_conn = (struct espconn *)os_malloc((uint32)sizeof(struct espconn));
 esp_conn->type = ESPCONN_TCP;
 esp_conn->state = ESPCONN_NONE;
 esp_conn->proto.tcp = (esp_tcp *)os_malloc((uint32)sizeof(esp_tcp));
 esp_conn->proto.tcp->local_port = MCPESP_SERVER_PORT;
 info_con = espconn_accept(esp_conn);

 time0 = millis();
 #ifdef _DEBUG_COMUNICACION_CONEXION
   debug.print("[CTCP] Código de arranque: ");
   debug.println(info_con);

   while(info_con != ESPCONN_OK){
     yield();
     debug.print("[CTCP] Estableciendo servidor TCP. Tiempo requerido: ");
     debug.println(millis()-time0);
     info_con = espconn_accept(esp_conn);
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

 #ifdef _DEBUG_COMUNICACION_CONEXION
   debug.println("SERVICIO TCP: Establecido.");
 #endif

}

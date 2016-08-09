/*

 ESPWifi.c - Funciones de comunicacion para la comunicación TCP entre dispositivos ESP8266 - ESP8266.

 */

 /******************************************************************************
  * FunctionName : tcp_server_listen
  * Description  : TCP server listened a connection successfully
  * Parameters   : arg -- Additional argument to pass to the callback function
  * Returns      : none
  *******************************************************************************/
void configWifi(){

    int8_t numwifi;
    String ssidscan;
    struct station_config *config = (struct station_config *)malloc(sizeof(struct
    station_config));

    wifi_station_get_config_default(config);

#ifdef _DEBUG_WIFI
    for(int i = 0; i < 15; i++){
      debug.print((char)config->ssid[i]);
    }
    debug.println();
#endif

    // Si el primer caracter del ssid guardado en la flash, indica que se ha realizado un
    // reset del dispositivo.
    if (config->ssid[2] != 'P'){

      // Se establece modo Acces Point
      wifi_set_opmode(STATION_MODE);
      //WiFi.mode(WIFI_STA);
      //WiFi.disconnect();
      delay(100);

      // Realizar condicional si no hay datos en memoria se buscan nuevas redes.

      // Se determina el número de redes disponibles
      do {
        numwifi = 0;
        // Se determina el número de redes visibles
        do {
          numwifi = WiFi.scanNetworks();
  #ifdef _DEBUG_WIFI
          debug.print("Numero de redes encontradas: ");
          debug.println(numwifi);
  #endif
          delay(1000);
        } while (!numwifi);

        // Se identifican las redes visibles, y comprueba si ssid presenta el prefijo
        // predefinido.
        for (int i = 0; i < numwifi; ++i)
        {
          ssidscan = WiFi.SSID(i);
  #ifdef _DEBUG_WIFI
          debug.print("Buscando red cliente. Encontrada: ");
          debug.println(ssidscan);
          debug.println(ssidscan.indexOf(PRE_SSID));
          delay(200);
  #endif
          if(ssidscan.indexOf(PRE_SSID) >= 0){
            numwifi = -1;
  #ifdef _DEBUG_WIFI
          debug.print("Red encontrada: ");
          debug.print(ssidscan.indexOf(PRE_SSID));
          debug.print(" Longitud:");
          debug.println(ssidscan.length());
  #endif
          break;
          }
          delay(50);
        }

    } while(numwifi != -1);

    // Se convierte la variable ssidscan de String to char.
    uint8_t len = ssidscan.length();
    char ssid[len];
    ssidscan.toCharArray(ssid,len+1);

    // Se indica que la red wifi no se encuentra registrada en memoria.
    strcpy(reinterpret_cast<char*>(config->ssid), "MCPES");
    if (strcmp(reinterpret_cast<const char*>(config->ssid), ssid) == 0){
      reset_wifi = false;
      strcpy(reinterpret_cast<char*>(config->password), CONTRASENA);
#ifdef _DEBUG_WIFI
      debug.println("Red guardada previamente.");
#endif
    }
    else {
      reset_wifi = true;
#ifdef _DEBUG_WIFI
      debug.println("Red no guardada previamente.");
#endif
    }

    strcpy(reinterpret_cast<char*>(config->ssid), ssid);
    strcpy(reinterpret_cast<char*>(config->password), CONTRASENA);
    config->bssid_set = 0;

  #ifdef _DEBUG_WIFI
    debug.print("SSID: ");
    debug.println(ssidscan);
    for(int i = 0; i < 15; i++){
      debug.print((char)config->ssid[i]);
    }
    debug.println("\nPASSWORD");
    for(int i = 0; i < 12; ++i){
      debug.print((char)config->password[i]);
    }
  #endif
    // Las nuevas modificaciones son grabadas en la memoria flash.
    wifi_station_set_config(config);

  }
  os_free(config);

  //WiFi.enableSTA(true);

  // Se establece la conexión con la red.
//  ETS_UART_INTR_DISABLE();
  wifi_station_connect();
//  ETS_UART_INTR_ENABLE();
  wifi_set_channel(0);
  wifi_station_dhcpc_start();

  delay(2000);

#ifdef _DEBUG_WIFI
  debug.println("Conectando");
  unsigned long t1 = millis();
#endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1);
  }
#ifdef _DEBUG_WIFI
  debug.print("Conectado. Tiempo requerido: ");
  debug.print(millis()-t1);
#endif

#ifdef _DEBUG_WIFI
  debug.println("");
  debug.println("WiFi connected");
  debug.println("IP address: ");
  debug.println(WiFi.localIP());
#endif

  // Indicar de conexión Wifi establecida
  digitalWrite(LED_BUILTIN,HIGH);

}

/**
 * Se comprueba si hay algún tipo de solicitud para conectarse al dispositivo.
 * El número máximo de clientes simultaneos es uno, todas las demás conexiónes
 * serán rechazadas.
 **/
void check_clientes(){
    // Se comprueba si hay solicitudes de clientes
    if (mcpesp_server.hasClient()){
        //  Si hay solicitudes de nuevos clientes.
        // Se comprueba si ya hay alguna conexión activa. En el supuesto de no haber
        // niguna conexión se establece.
        if (!cliente || !cliente.connected()){
            if(cliente) cliente.stop();
            cliente = mcpesp_server.available();
        }
        // En el supuesto de encontrarse una nueva solicitud de conexión, ésta es rechazada.
        WiFiClient rechazo_cliente = mcpesp_server.available();
        rechazo_cliente.stop();
    }

}

bool tcp_sent(uint8_t *pdata){

  unsigned long time0;
  int8_t res_envio;

  transmision_finalizada = false;
  time0 = millis();

//  res_envio = espconn_send(esp_conn, pdata , strlen(pdata));

  while (!transmision_finalizada){
    yield();

    if (res_envio != ESPCONN_OK)
       res_envio = espconn_send(esp_conn, pdata , strlen(reinterpret_cast<char*>(pdata)));

    #ifdef _DEBUG_COMUNICACION
    debug.print("COMUNICACION CLIENTE: Codigo de envio: ");
    debug.println(res_envio);
    #endif

    if ((millis()-time0)>MAX_ESPWIFI){
      return false;
    }
  }

  return true;
}

/**
 * Comprueba si el cliente con el que se ha establecido la conexión a transmitido
 * información.
 *
 **/
void comunicacion_cliente(){


  if (CMD == '$' || !transmision_finalizada)
    return;

  switch (CMD)
  {
    case USUARIO_REGISTRADO:
      #ifdef _DEBUG_COMUNICACION
      debug.print("COMUNICACION CLIENTE: Confirmacion de registro.");
      unsigned long time0;
      time0 = millis();
      #endif

      uint8_t psent[1];
      psent[0] = WACK;

      if (tcp_sent(psent))
        registrado  = true;
      else
        registrado = false;

       #ifdef _DEBUG_COMUNICACION
            debug.println("-----------------------------------------------");
            debug.print("CONFIRMACION: REGISTRADO. Tiempo requerido: ");
            debug.println(millis()-time0);
            debug.println("-----------------------------------------------");
       #endif
     break;

     case '!':
     #ifdef _DEBUG_COMUNICACION
         debug.println("COMUNICACION: !Soy el servidor 1.");
     #endif
         char psent2[18];
         sprintf(psent2, "!Soy el servidor 1");
         tcp_sent(reinterpret_cast<uint8_t*>(psent2));
      //   espconn_send(esp_conn, reinterpret_cast<uint8_t*>(&data_sent), 18);
       break;

     default:
     #ifdef _DEBUG_COMUNICACION
         debug.println("COMUNICACION: CMD no identificado.");
     #endif
     break;
   }

   CMD = '$';

}

void reset_configwifi(void *pArg){

  if(digitalRead(GPIO_SINC) == HIGH){

    struct station_config *config = (struct station_config *)malloc(sizeof(struct
    station_config));

    // Se desconecta la red wifi para grabar los parámetros de configuración.
    //WiFi.disconnect();
    wifi_station_disconnect();

    // Se obtiene la parámetros de red guardados en la Flash.
    wifi_station_get_config(config);

    strcpy(reinterpret_cast<char*>(config->ssid), "RESET");
    strcpy(reinterpret_cast<char*>(config->password), "RESET");

    // Las nuevas modificaciones son grabadas en la memoria flash.
    wifi_station_set_config(config);
    os_free(config);
    os_free(timerrest);

#ifdef _DEBUG_WIFI
    debug.println("Parametros de conexión borrados.");
    debug.println("Ahora se realiza un reset global del dispositivo.");
#endif

    //ESP.restart();
    system_restart();

  }

}

void isrsinc(){

  ets_intr_lock();
  // Se ha pulsado el pulsador se sincronización, se busca el nuevo usuario he
  // indica que se ha conectado correctamente a la red.
  if(digitalRead(GPIO_SINC) == LOW){

    timerrest = (os_timer_t*)os_malloc(sizeof(os_timer_t));

    // Se activa un timer, si transcurrido un tiempo el botón sigue pulsado,
    // se borran los paremtros de la configuración wifi.
    os_timer_setfn(timerrest,reset_configwifi, NULL);

    // Se activa el timer de sincronización
    os_timer_arm(timerrest, TIEMPO_RESET, false);

#ifdef _DEBUG_WIFI
    debug.println("Activacion timer de reset parametros Wifi.");
#endif

  }

  ets_intr_unlock();

}

/**
**/
int8_t confirmar_conexion(){

  int8_t res_envio;
  unsigned long time0;
  uint8_t psent[1];

  // Espera
#ifdef _DEBUG_WIFI
    Serial.println("CONFIRMACION: Esperando conexión de un cliente.");
#endif

  time0 = millis();
  while (!tcp_establecido) {
    yield();
    if ((millis()-time0)>MAX_ESPWIFI){
      return false;
    }
  }

#ifdef _DEBUG_WIFI
    Serial.println("CONFIRMACION: Conexion establecida. Tiempo requerido: ");
#endif

  // Terminar
  if (tcp_establecido){

    transmision_finalizada = false;

#ifdef _DEBUG_WIFI
    Serial.print("CONFIRMACION: Conexion establecida. Tiempo requerido: ");
    Serial.println(millis()-time0);
#endif

#ifdef _DEBUG_WIFI
    Serial.println("CONFIRMACION: Esperando respuesta de conexion de red.");
#endif
    time0 = millis();
    while (CMD != USUARIO_REGISTRADO) {
      yield();
      if ((millis()-time0)>MAX_ESPWIFI){
        return false;
      }
    }
    CMD = '$';
    psent[0] = WACK;
    time0 = millis();

    if (!tcp_sent(psent))
      return false;

/*
 #ifdef _DEBUG_COMUNICACION
   res_envio = espconn_send(esp_conn, psent , 1);

   while (!transmision_finalizada){
      yield();

      if (res_envio != ESPCONN_OK)
         res_envio = espconn_send(esp_conn, psent , 1);

      debug.print("CONFIRMACION: Codigo de envio: ");
      debug.println(res_envio);

      if ((millis()-time0)>MAX_ESPWIFI){
        return false;
      }
    }
 #else
     res_envio = espconn_send(esp_conn, psent , 1);

     while (!transmision_finalizada){
        yield();

        if (res_envio != ESPCONN_OK)
           res_envio = espconn_send(esp_conn, psent , 1);

        if ((millis()-time0)>MAX_ESPWIFI){
          return false;
        }
      }
 #endif
*/
 #ifdef _DEBUG_COMUNICACION
      debug.println("-----------------------------------------------");
      debug.print("CONFIRMACION: REGISTRADO. Tiempo requerido: ");
      debug.println(millis()-time0);
      debug.println("-----------------------------------------------");
 #endif
    registrado = true;
    return true;
  }

  #ifdef _DEBUG_COMUNICACION
       debug.println("-----------------------------------------------");
       debug.print("CONFIRMACION: NO REGISTRADO. Tiempo requerido: ");
       debug.println(millis()-time0);
       debug.println("-----------------------------------------------");
  #endif
  return false;
}


/******************************************************************************
 * FunctionName : tcp_server_sent_cb
 * Description  : data sent callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
 *******************************************************************************/
void tcp_server_sent_cb(void *arg)
{
  //data sent successfully
  transmision_finalizada = true;
#ifdef _DEBUG_COMUNICACION
  debug.println("TCP: TRANSMISION REALIZADA CORRECTAMENTE");
#endif
}

/******************************************************************************
 * FunctionName : tcp_server_discon_cb
 * Description  : disconnect callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
 *******************************************************************************/
void tcp_server_discon_cb(void *arg)
{
  //tcp disconnect successfully
#ifdef _DEBUG_COMUNICACION
  debug.println("TCP: DESCONEXION REALIZADA CORRECTAMENTE");
#endif
  tcp_desconectado = true;
  tcp_establecido = false;
  transmision_finalizada = true;
}

/******************************************************************************
 * FunctionName : tcp_server_recon_cb
 * Description  : reconnect callback, error occured in TCP connection.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
 *******************************************************************************/
void tcp_server_recon_cb(void *arg, sint8 err)
{
    //error occured , tcp connection broke.
    tcp_desconectado = true;
    tcp_establecido = false;
    transmision_finalizada = true;
    #ifdef _DEBUG_COMUNICACION
        debug.println("TCP: CONEXION INTERRUMPIDA. CODIGO DE ERROR: ");
        debug.println(err);
    #endif
}

/******************************************************************************
 * FunctionName : tcp_server_recv_cb
 * Description  : receive callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
 *******************************************************************************/
void tcp_server_recv_cb(void *arg, char *tcp_data, unsigned short length)
{
    //received some data from tcp connection
    struct espconn *pespconn = static_cast<struct espconn *> (arg);

    digitalWrite(2, !digitalRead(2));

#ifdef _DEBUG_COMUNICACION_LIMIT
    debug.print("TCP: Usuarios conectados. A espera de datos: ");
    debug.println(tcp_data);
#endif

    /* PROCESAMIENTO DE LA INFORMACIÓN RECIBIDA */
    switch (tcp_data[0]) {
        case 'T':
      //      cliente.write(MCPread(VOLTAGE_RMS,BVOLTAGE_RMS));
            break;
        case 'C':
      //      cliente.write(MCPread(CURRENT_RMS,BCURRENT_RMS));
            break;
        case '!':
        #ifdef _DEBUG_COMUNICACION
            debug.println("TCP: Comunicacion !");
        #endif
            CMD = tcp_data[0];
            break;

        case USUARIO_REGISTRADO:
        #ifdef _DEBUG_COMUNICACION
            debug.println("TCP: Proceso de registro");
        #endif
            CMD = tcp_data[0];
            break;

        case '#':
            tcp_finalizar = true;
            break;

        default:
            #ifdef _DEBUG_COMUNICACION
            Serial.println("TCP: No identificado.");
            // Nothing yet
            #endif
            break;
    }

}

/******************************************************************************
 * FunctionName : tcp_server_listen
 * Description  : TCP server listened a connection successfully
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
 *******************************************************************************/
void tcp_listen(void *arg)
{
#ifdef _DEBUG_COMUNICACION
      debug.println("TCP: Comunicacion iniciada");
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
//    tcp_server_multi_send();

   tcp_establecido = true;
   tcp_desconectado = false;
   transmision_finalizada = true;

}

void servidor_tcp(){

  int8_t res_con;

#ifdef _DEBUG_COMUNICACION_CONEXION
  debug.println("SERVICIO TCP: Arranque del servidor tcp.");
  unsigned long time0 = millis();
#endif

  esp_conn = (struct espconn *)os_malloc((uint32)sizeof(struct espconn));

  esp_conn->type = ESPCONN_TCP;
  esp_conn->state = ESPCONN_NONE;
  esp_conn->proto.tcp = (esp_tcp *)os_malloc((uint32)sizeof(esp_tcp));
  esp_conn->proto.tcp->local_port = MCPESP_SERVER_PORT;
  sint8 ret = espconn_accept(esp_conn);

  espconn_regist_time(esp_conn, TCP_TIEMPO_CONEXION, 1);
  espconn_regist_connectcb(esp_conn, tcp_listen);


  debug.println("espconn_accept: ");
  debug.println(ret);

#ifdef _DEBUG_COMUNICACION_CONEXION
  res_con = espconn_accept(esp_conn);

  debug.print("SERVICIO TCP: Aviso de arranque: ");
  debug.println(res_con);

  time0 = millis();
  while(res_con != ESPCONN_OK){
    debug.print("SERVICIO TCP: Estableciendo servidor TCP. Tiempo requerido: ");
    debug.println(millis()-time0);

    yield();

    res_con = espconn_accept(esp_conn);

/*    if ((millis()-time0)>MAX_ESPWIFI){
      return;
    }
*/  }
#else
//  time0 = millis();
  while(espconn_accept(esp_conn) != ESPCONN_OK ){
    yield();
/*    if ((millis()-time0)>MAX_ESPWIFI){
      return;
    }
*/  }
#endif

#ifdef _DEBUG_COMUNICACION_CONEXION
    debug.println("SERVICIO TCP: Establecido.");
#endif

}

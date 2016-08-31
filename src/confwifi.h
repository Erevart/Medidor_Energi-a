/*

 confwifi.c - Funciones para la configurción de la conexion wifi.

 */

 /******************************************************************************
  * Función : wifi_station_scan_done
  * @brief  : Realiza una escaneo de las redes inalambricas cercanas y se conecta a aquellas
                   cuyo nombre empiece por el prefijo "MCPESP_".
  * @param  : arg - puntero a la estrcutura de datos que indica las redes detectadas.
  * @param  : status - variable que indica si la escaneo de redes ha sido realizado
  *            con éxito
  * @return : none
  * Etiqueta debug : Todos los comentarios para depuración de esta función
                    estarán asociados a la etiqueta: "SCAN".
 *******************************************************************************/
 void wifi_station_scan_done(void *arg, STATUS status)
{

  if (status == OK){

    char cmpssid[7];
    struct bss_info *bss_link = (struct bss_info *)arg;

    // Se identifican las redes visibles, y comprueba si el SSID presenta el
    // prefijo predefinido.
    while (bss_link != NULL){
      #ifdef _DEBUG_WIFI
        debug.print("[SCAN] Buscando red cliente. Encontrada: ");
        debug.println((char*) bss_link->ssid);
      #endif
      memcpy(cmpssid, bss_link->ssid, 7);
      if(os_strcmp(cmpssid,PRE_SSID) == 0){

        struct station_config *config = (struct station_config *)malloc(sizeof(struct
        station_config));

        // Se lee de memoria la configuración definida en sesiones anteriores
        wifi_station_get_config_default(config);

        #ifdef _DEBUG_WIFI
          debug.print("[SCAN] Red encontrada: ");
          debug.print((char*)bss_link->ssid);
          debug.print(" Longitud:");
          debug.println(os_strlen((char*)bss_link->ssid));
        #endif

        // Se comprueba si la red detectada ya fue registrada previamente.
        os_strcpy(reinterpret_cast<char*>(config->ssid), "MCPES");
        if (os_strcmp(reinterpret_cast<const char*>(config->ssid), (char*)bss_link->ssid) == 0){
          reset_wifi = false;
          os_strcpy(reinterpret_cast<char*>(config->password), CONTRASENA);
          #ifdef _DEBUG_WIFI
            debug.println("[SCAN] Red guardada previamente.");
          #endif
        }
        else {
          reset_wifi = true;
          #ifdef _DEBUG_WIFI
            debug.println("[SCAN] Red no guardada previamente.");
          #endif
        }

        os_strcpy(reinterpret_cast<char*>(config->ssid), (char*)bss_link->ssid);
        os_strcpy(reinterpret_cast<char*>(config->password), CONTRASENA);
        config->bssid_set = 0;

        #ifdef _DEBUG_WIFI
          debug.print("[SCAN] SSID: ");
          debug.println((char*)bss_link->ssid);
          for(int i = 0; i < 15; i++){
            debug.print((char)config->ssid[i]);
          }
          debug.println("\n[CONFW] PASSWORD");
          for(int i = 0; i < 12; ++i){
            debug.print((char)config->password[i]);
          }
        #endif

        // Las nuevas modificaciones son grabadas en la memoria flash.
        wifi_station_set_config(config);

        os_free(config);

        estadoscan = true;
        return;
      }
      bss_link = bss_link->next.stqe_next;
    }
  }
  #ifdef _DEBUG_WIFI
    else {
        debug.println("[SCAN] Escaneo fallido. ");
    }
  #endif

   estadoscan = false;
}


 /******************************************************************************
  * Función : configWifi
  * @brief  : Realiza una escaneo de las redes inalambricas cercanas y se conecta a aquellas
                   cuyo nombre empiece por el prefijo "MCPESP_".
  * @param  : none
  * @return : none
  * Etiqueta debug : Todos los comentarios para depuración de esta función
                    estarán asociados a la etiqueta: "CONFW".
  *******************************************************************************/
void configWifi(){

    int8_t numwifi;
    String ssidscan;
    struct station_config *config = (struct station_config *)malloc(sizeof(struct
    station_config));

    // Se establece modo AP.
    wifi_set_opmode(STATION_MODE);
    delay(100);

    // Se lee de memoria la configuración definida en sesiones anteriores
    wifi_station_get_config_default(config);

    #ifdef _DEBUG_WIFI
      debug.print("[CONFW] SSID guardado en memoria: ");
      for(int i = 0; i < 15; i++){
        debug.print((char)config->ssid[i]);
      }
      debug.println();
    #endif

    // Si el primer caracter del ssid guardado en la flash, indica que se ha realizado un
    // reset del dispositivo. Se realiza la búsqueda de nuevas redes inalambricas.
    if (config->ssid[2] != 'P'){

      struct scan_config scanconfig;

      os_memset(&scanconfig, 0, sizeof(scanconfig));

      do {
        if (estadoscan == false){
          wifi_station_scan(&scanconfig, wifi_station_scan_done);
          estadoscan = -1;
        }
        yield();
      } while (estadoscan != true);

  }

  os_free(config);

  // Se realiza la conexión a la red seleccionada.
  wifi_station_connect();
  wifi_set_channel(0);

  // Se inicia el servicio dhcp.
  wifi_station_dhcpc_start();

  delay(2000);

  #ifdef _DEBUG_WIFI
    debug.println("[CONFW] Conectando");
    unsigned long t1 = millis();
  #endif
    while (WiFi.status() != WL_CONNECTED) {
      yield();
    }
  #ifdef _DEBUG_WIFI
    debug.print("[CONFW] Conectado. Tiempo requerido: ");
    debug.print(millis()-t1);
  #endif

  #ifdef _DEBUG_WIFI
    debug.println();
    debug.println("[CONFW] WiFi connected");
    debug.println("[CONFW] IP address: ");
    debug.println(WiFi.localIP());
  #endif

  // Indicar de conexión Wifi establecida
  digitalWrite(LED_BUILTIN,HIGH);

}


/******************************************************************************
 * Función : reset_configwifi
 * @brief  : Realiza la desconexión de la red inalambrica a la cual se encuentra
            y borra la información correspondiente a ésta de la memoria flash.
 * @param  : pArg - puntero que indica el timer que ha disparado la interrupción.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "RST_CONFW".
 *******************************************************************************/
void reset_configwifi(void *pArg){

  if(digitalRead(GPIO_SINC) == HIGH){

    struct station_config *config = (struct station_config *)malloc(sizeof(struct
    station_config));

    // Se desconecta de la red wifi para grabar los parámetros de configuración.
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
      debug.println("[RST_CONFW] Parametros de conexión borrados.");
      debug.println("[RST_CONFW] Ahora se realiza un reset global del dispositivo.");
    #endif

    system_restart();

  }

}

/******************************************************************************
 * Función : isrsinc
 * @brief  : Interrupción disparada cuando el boton de reset es pulsado. Activa
             el timer de reset para desconectarse de la red asociada, borrar su
             información y reiniciar el dispositivo.
 * @param  : none
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "ISR".
 *******************************************************************************/
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
      debug.println("[ISR] Activacion timer de reset parametros Wifi.");
    #endif

  }

  ets_intr_unlock();

}

/******************************************************************************
 * Función : confirmar_conexion
 * @brief  : Solicita el registro del dispositivo en la red selecionada.
 * @param  : none
 * @return : true - El dispositivo ha sido registrado correctamente.
 * @return : false - El dispositivo no ha sido registrado.
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "CNF_CNX".
 *******************************************************************************/
int8_t confirmar_conexion(){

  unsigned long time0;
  char psent[3];

  #ifdef _DEBUG_WIFI
    Serial.println("[CNF_CNX] Esperando conexión de un cliente.");
  #endif

  // Se espera a establecer un canal de comunicacion TCP para iniciar el proceso
  // de registro.
  time0 = millis();
  while (!tcp_establecido) {
    yield();
    if ((millis()-time0)>MAX_ESPWIFI){
      return false;
    }
  }

  #ifdef _DEBUG_WIFI
      Serial.print("[CNF_CNX] Conexion establecida. Tiempo requerido: ");
      Serial.println(millis()-time0);
      Serial.println("[CNF_CNX] Esperando respuesta de conexion de red.");
  #endif

  // Se espera la verificación de registro por parte del cliente.
  time0 = millis();
  while (CMD != USUARIO_REGISTRADO) {
    yield();
    if ((millis()-time0)>MAX_ESPWIFI){
      return false;
    }
  }

  // Dispositivo registrado, se indica que la operación de registro ya ha sido
  // realizada.
  CMD = '$';

  // Se confirma la recepción de la validación de registro del dispositivo.
  psent[0] = TCP_START;
  psent[1] = 1;
  psent[2] = WACK;

  #ifdef _DEBUG_COMUNICACION
    time0 = millis();
  #endif

  if (!tcp_sent(reinterpret_cast<uint8_t*>(psent))){
    #ifdef _DEBUG_COMUNICACION
     debug.println("-----------------------------------------------");
     debug.print("[CNF_CNX] NO REGISTRADO. Tiempo requerido: ");
     debug.println(millis()-time0);
     debug.println("-----------------------------------------------");
    #endif
    return false;
  }

  // Se actualiza el contador de tiempo.
  update_rtc_time(true);

 #ifdef _DEBUG_COMUNICACION
      debug.println("-----------------------------------------------");
      debug.print("[CNF_CNX] REGISTRADO. Tiempo requerido: ");
      debug.println(millis()-time0);
      debug.println("-----------------------------------------------");
 #endif
    registrado = true;
    return true;
}

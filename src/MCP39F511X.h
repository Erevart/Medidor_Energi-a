/*

 MCP39F511X.h - MCP uart comunication function

 */

 #include <Arduino.h>

 /* Códigos de error */
 #define NOCOMUNICACION  0x01  // No hay comunicacion. No se recibe respuesta por parte del receptor
 #define CHECKSUMERR     0x02  // La informacion recibida no es correcta. (Fallo checksum)
 #define ENACK           0x03  // Se ha recibido un NACK en la confirmación de la respuesta. (Posible comando incorrecto)
 #define ECSFAIL         0x04  // Se ha recibido un CSFAIL en la confirmación de la respuesta. (Comprobar checksum)


 /* Instrucciones MCP39F511 & MCP39F511N  (ver Datasheet) */
 // Bytes de control
 #define ID_BYTE 0x03
 #define MCP_ID 0xA5
 #define ID_BYTE 0x03
 #define ACK 0x06
 #define NACK 0x15
 #define CSFAIL 0x51
 // Bytes de instruccion
 #define MCP_READ 0x4E
 #define MCP_WRITE 0x4D
 #define MCP_SETAP 0x41
 #define MCP_SAVE2FLASH 0X53
 #define MCP_READ_EEPROM 0x42
 #define MCP_WRITE_EEPROM 0x50
 #define MCP_ERASE_EEPROM 0x4F
 #define MCP_AUTO_GAIN 0x5A
 #define MCP_AUTO_REACTIVE 0x7A
 #define MCP_AUTO_FREQUENCY 0x76

 /*********************************/
 /* Definicion de registros */
 /*********************************/
// Array de datos
const uint16_t ADDR[61] = {0x0000,0x0002,0x0004,0x0006,0x0008,0x000A,0x000C,0x000E,0x0012,
   0x0016,0x001A,0x001E,0x0022,0x0026,0x002A,0x002E,0x0036,0x003E,0x0046,0x004E,
   0x0056,0x005E,0x0066,0x006E,0x0070,0x0070,0x0074,0x0076,0x0078,0x007A,0x007C,
   0x007E,0x0080,0x0084,0x0088,0x008C,0x0090,0x0094,0x0098,0x009A,0x009C,0x009E,
   0x00A0,0x00A4,0x00A8,0x00AA,0x00AC,0x00AE,0x00B2,0x00B6,0x00BA,0x00BE,0x00C2,
   0x00C6,0x00CA,0x00CE,0x00D0,0x00D2,0x00D6,0x00DA,0x00DE};

const uint8_t TYPE_DATA[61] = {0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x04,0x04,0x04,0x04,
   0x04,0x04,0x04,0x04,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x02,0x02,0x02,
   0x02,0x02,0x02,0x02,0x02,0X02,0x04,0x04,0x04,0x04,0x04,0x04,0x02,0x02,0x02,
   0x02,0x04,0x04,0x02,0x02,0x02,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x02,
   0x02,0x04,0x04,0x04,0x04};

 // enumu
 enum REGISTROS  {
    INSTRUCTION_POINTER,
    SYSTEM_STATUS,
    SYSTEM_VERSION,
    VOLTAGE_RMS,
    LINE_FREQUENCY,
    POWER_FACTOR1,
    POWER_FACTOR2,
    CURRENT_RMS1,
    CURRENT_RMS2,
    ACTIVE_POWER1,
    ACTIVE_POWER2,
    REACTIVE_POWER1,
    REACTIVE_POWER2,
    APPARENT_POWER1,
    APPARENT_POWER2,
    IMPORT_ENERGY_ACTIVE_COUNTER1,
    IMPORT_ENERGY_ACTIVE_COUNTER2,
    EXPORT_ENERGY_ACTIVE_COUNTER1,
    EXPORT_ENERGY_ACTIVE_COUNTER2,
    IMPORT_ENERGY_REACTIVE_COUNTER1,
    IMPORT_ENERGY_REACTIVE_COUNTER2,
    EXPORT_ENERGY_REACTIVE_COUNTER1,
    EXPORT_ENERGY_REACTIVE_COUNTER2,
    // Registro de calibración
    CALIBRATION_REGISTERS_DELIMITER,
    GAIN_CURRENT_RMS1,
    GAIN_CURRENT_RMS2,
    GAIN_VOLTAGE_RMS,
    GAIN_ACTIVE_POWER1,
    GAIN_ACTIVE_POWER2,
    GAIN_REACTIVE_POWER1,
    GAIN_REACTIVE_POWER2,
    GAIN_LINE_FREQUENCY,
    OFFSET_CURRENT_RMS1,
    OFFSET_CURRENT_RMS2,
    OFFSET_ACTIVE_POWER1,
    OFFSET_ACTIVE_POWER2,
    OFFSET_REACTIVE_POWER1,
    OFFSET_REACTIVE_POWER2,
    PHASE_COMPENSATION1,
    PHASE_COMPENSATION2,
    APPARENT_POWER_DIVISOR1,
    APPARENT_POWER_DIVISOR2,
    // Registros de configuración
    SYSTEM_CONFIGURATION,
    EVENT_CONFIGURATION,
    ACCUMULATION_INTERVAL_PARAMETER,
    CALIBRATION_VOLTAGE,
    CALIBRATION_LINE_FREQUENCY,
    RANGE1,
    CALIBRATION_CURRENT1,
    CALIBRATION_POWER_ACTIVE1,
    CALIBRATION_POWER_REACTIVE1,
    RANGE2,
    CALIBRATION_CURRENT2,
    CALIBRATION_POWER_ACTIVE2,
    CALIBRATION_POWER_REACTIVE2,
    VOLTAGE_SAG_LIMIT,
    VOLTAGE_SURGE_LIMIT,
    OVER_CURRENT1_LIMIT,
    OVER_CURRENT2_LIMIT,
    OVER_POWER1_LIMIT,
    OVER_POWER2_LIMIT,
   // Ultimo registro
   LAST_REGISTER
 };



/* Prototipo de Funciones
void error(uint8_t code);
uint8_t checkACK();
uint8_t Getchecksum(uint8_t *frame);
uint8_t _MCPwrite(uint16_t *reg, uint8_t *num_bytes, uint32_t *dato);
void MCPwrite(uint16_t reg, uint8_t num_bytes, uint32_t dato);
uint32_t _MCPread(uint16_t *reg, uint8_t *num_bytes);
uint32_t MCPread(uint16_t reg, uint8_t num_bytes);
void MCPsetap(uint16_t reg);
void MCPsaveflash();
void MCPeraseeprom();
void MCPautogain(uint8_t parameter);
*/


/*

 MCP39F511X.c - MCP uart comunication function

 */


/******************************************************************************
 * Función : error
 * @brief  : Avisa e indica del error producido durante la comunicación
 * @param  : code - identificador del error producido.
 * @return : none
 * Etiqueta debug : Todos los comentarios para depuración de esta función
                   estarán asociados a la etiqueta: "ERROR".
 *******************************************************************************/
void error(uint8_t code){

    String err = "[ERROR] No se ha producido ningun error.";

    digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));

    switch (code){
      case NOCOMUNICACION: err ="[ERROR] no hay comunicacion. No se recibe respuesta por parte del receptor.";                  break;
      case CHECKSUMERR:    err ="[ERROR] la informacion recibida no es correcta. (Fallo checksum)";                             break;
      case ENACK:          err ="[ERROR] se ha recibido un NACK en la confirmación de la respuesta. (Posible comando incorrecto)"; break;
      case ECSFAIL:        err ="[ERROR] se ha recibido un CSFAIL en la confirmación de la respuesta. (Comprobar checksum)"; break;
    }

#ifdef _DEBUG_ERROR
    debug.println(err);
#endif
}

/******************************************************************************
 * Función : checkACK
 * @brief  : Comprueba la confirmación de la recepción de los datos por parte del
 *           otro lado de la comunicación.
 * @param  : none
 * @return : NACK - No se recibe respuesta por parte del otro dispositivo,
 *                  si este comunica un fallo en la comunicación o indica NACK
 * @return : ACK - La recepsión ha sido confirmada correctamnte.
 *******************************************************************************/
uint8_t checkACK(){

  uint8_t check;

  // Se espera el byte de confirmación de recepción.
  // Si transcurre un periodo superior a 500ms se considera que la comunicacion se ha perdido o nunca
  // se estableció.
  unsigned long prevtime = millis();
  while (!(uart.available()>= 1))
    if ((millis()-prevtime)>= 1000){
      // Informar de error en la comunicacion.
    #ifdef _DEBUG_ERROR
      error(NOCOMUNICACION);
    #endif
      return NACK;
    }

  check = uart.read();

  if (check == NACK){

    #ifdef _DEBUG_ERROR
      error(ENACK);
    #endif

    return NACK;
  }
  else if(check == CSFAIL){

    #ifdef _DEBUG_ERROR
      error(ECSFAIL);
    #endif

    //return CSFAIL
    return NACK;
  }

  return check;
}

/******************************************************************************
 * Función : Getchecksum
 * @brief  : Devuelve el código de verificación de trama según las indicaciones
 *             del fabricante
 * @param  : frame - puntero al array que contiene la trama de datos.
 * @return : devuelve el checsum correspondiente.
 *******************************************************************************/
uint8_t Getchecksum(uint8_t *frame){

  uint16_t checksum = 0;

  for (uint8_t i = 0; i <= frame[1]-2; i++){
    checksum += frame[i];
  }
  return (uint8_t) checksum;

}

/******************************************************************************
 * Función : _MCPwrite
 * @brief  : Escribe a partir del registro indicado el número de bytes indicados.
 * @param  : reg - puntero a la variable que indica el registro a escribir.
 * @param  : num_bytes - puntero a la variable que indica el número de bytes a escribir.
 * @param  : dato - puntero al array de datos a enviar.
 * @return : devuelve si la operación de escritura se ha realizado correctamente.
 *******************************************************************************/
uint8_t _MCPwrite(uint16_t *reg, uint8_t *num_bytes, uint32_t *dato){

  uint8_t frame[8+*num_bytes];
  uint32_t readregister = 0;

  //  Se registra la trama de datos que se desea enviar.
  frame[0] = MCP_ID;
  frame[1] = 0x08 + *num_bytes;
  frame[2] = MCP_SETAP;
  frame[3] = *reg >> 8 ;
  frame[4] = *reg;
  frame[5] = MCP_WRITE;
  frame[6] = *num_bytes;

  // Número de datos fijos 0x06
  // BIG ENDIAN
  for (uint8_t i = 0x07; i <= 0x06 + *num_bytes; i++){
    frame[i] = *dato >> ((*num_bytes) + 0x06 - i)*8;
  }

  frame[frame[1]-1] = Getchecksum(frame);

//  uart.write(frame,frame[1]);
  for (uint8_t i = 0; i <= frame[1]-1; i++){
    uart.write(frame[i]);
    delay(1);
    #ifdef _DEBUBG_TX
      debug.write(frame[i]);
    #endif
  }

  return checkACK();
}

  /******************************************************************************
   * Función : MCPwrite
   * @brief  : Escribe a partir del registro indicado el número de bytes indicados,
   *            asegurando que ésta se ha realizado adecuadamente. En caso de producirse un error
   *            reintenta realizar la operación un número máximo de MAX_INTENTOS.
   * @param  : reg - variable que indica el registro a escribir.
   * @param  : num_bytes - variable que indica el número de bytes a escribir.
   * @param  : dato - array de datos a enviar.
   * @return : devuelve si la operación de escritura se ha realizado correctamente.
   *******************************************************************************/
void MCPwrite(uint16_t reg, uint8_t num_bytes, uint32_t dato){

  uint8_t check = NACK;     // Variable de comprobaciación para saber si la escrtura
                          // se ha realizado correctamente.
  uint8_t intentos = 0;   // Varible que indica el número de veces que se realiza
                          // la escritura.

  do {
    check = _MCPwrite(&reg,&num_bytes,&dato);
    intentos++;
  } while (check == NACK && intentos <= MAX_INTENTOS);

  return;
}

/******************************************************************************
 * Función : _MCPread
 * @brief  : Lee a partir del registro indicado el número de bytes solicitados.
 * @param  : reg - puntero a la variable que indica el registro a escribir.
 * @param  : num_bytes - puntero a la variable que indica el número de bytes a escribir.
 * @return : devuelve una uint32_t con la información almacenada en la posición indicada.
 *******************************************************************************/
uint32_t _MCPread(uint16_t *reg, uint8_t *num_bytes){

  uint8_t frame[8];
  uint32_t readregister = 0;

  //  Se registra la trama de datos que se desea enviar.
  frame[0] = MCP_ID;
  frame[1] = 0x08; // El número de bytes para esta funcion es fijo.
  frame[2] = MCP_SETAP;
  frame[3] = *reg >> 8 ;
  frame[4] = *reg;
  frame[5] = MCP_READ;
  frame[6] = *num_bytes;

  frame[frame[1]-1] = Getchecksum(frame);

//  uart.write(frame,frame[1]);

  for (uint8_t i = 0; i <= frame[1]-1; i++){
    uart.write(frame[i]);
    delay(1);
  }

  if (checkACK() == NACK){
    return NACK;
  }
  else{
    frame[0] = ACK;
    frame[1] = uart.read();
  }

  // Se realiza la lectura de los datos recibidos
  #ifdef _BIG_ENDIAN
    for (uint8_t i = 0; i < (frame[1]-3); i++){
      // El -3 es debio a que ya se han leido los 2 primeros valores, y el checksum no se va a leer.
      frame[2+i] = uart.read();
      readregister = readregister | (frame[2+i] << (frame[1]-4-i)*8); // (frame[1]-3-(i+1))
      delay(1);
      #ifdef _DEBUBG_RX
          debug.write(readregister);
      #endif
    }
  #else
    for (uint8_t i = 0; i < (frame[1]-3); i++){
      // El -3 es debio a que ya se han leido los 2 primeros valores, y el checksum no se va a leer.
      frame[2+i] = uart.read();
      readregister = readregister | (frame[2+i] << i*8); //
      delay(1);
      #ifdef _DEBUBG_RX
          debug.write(readregister);
      #endif
    }
  #endif

  if (Getchecksum(frame) == uart.read())
    return readregister;
#ifdef _DEBUG_ERROR
  else
    error(CHECKSUMERR);
#endif

  return -1;
}

/******************************************************************************
 * Función : MCPread
 * @brief  : Lee a partir del registro indicado el número de bytes solicitados,
 *           asegurando que ésta se ha realizado adecuadamente. En caso de producirse un error
 *           reintenta realizar la operación un número máximo de MAX_INTENTOS.
 * @param  : reg - puntero a la variable que indica el registro a escribir.
 * @param  : num_bytes - puntero a la variable que indica el número de bytes a escribir.
 * @return : devuelve una uint32_t con la información almacenada en la posición indicada.
 *******************************************************************************/
uint32_t MCPread(uint16_t reg, uint8_t num_bytes){

  uint32_t check = NACK;  // Variable de comprobaciación para saber si la escrtura
                          // se ha realizado correctamente. Al mismo tiempo, es la
                          // variable que contiene la información leida del MCP.
  uint8_t intentos = 0;   // Varible que indica el número de veces que se realiza
                          // la escritura.

  do {
    check = _MCPread(&reg,&num_bytes);
    intentos++;
  } while (check == NACK && intentos <= MAX_INTENTOS);

  return check;

}

 /******************************************************************************
  * Función : MCPsetap
  * @brief  : Modifica la dirección del puntero del MCP39F511X a la posición indicada.
  * @param  : reg - posición de memoria a la que se desea ubicar el puntero.
  * @return : none
  *******************************************************************************/
void MCPsetap(uint16_t reg){

  uint8_t frame[6];
  uint16_t checksum = 0;
  uint8_t intentos = 0;   // Varible que indica el número de veces que se realiza
                          // la escritura.

  //  Se registra la trama de datos que se desea enviar.
  frame[0] = MCP_ID;
  frame[1] = 0x06; // El número de bytes para esta funcion es fijo.
  frame[2] = MCP_SETAP;
  frame[3] = reg >> 8 ;
  frame[4] = reg;

  frame[frame[1]-1] = Getchecksum(frame);

  do {
    //uart.write(frame,frame[1]);
    for (uint8_t i = 0; i <= frame[1]-1; i++){
      uart.write(frame[i]);
      delay(1);
      #ifdef _DEBUBG_TX
        debug.write(frame[i]);
      #endif
    }
    intentos++;
  } while (checkACK() == NACK && intentos <= MAX_INTENTOS);

  return;
}

 /******************************************************************************
  * Función : MCPsaveflash
  * @brief  : Guarda las modificaciones realizadas en el MCP39F511X en su memoria flash
  * @param  : none
  * @return : none
  *******************************************************************************/
void MCPsaveflash(){

  uint8_t frame[4];
  uint8_t intentos = 0;   // Varible que indica el número de veces que se realiza
                          // la escritura.

  //  Se registra la trama de datos que se desea enviar.
  frame[0] = MCP_ID;
  frame[1] = 0x04; // El número de bytes para esta funcion es fijo.
  frame[2] = MCP_SAVE2FLASH;
  // El codigo de verificacion se calcula segun las indicaciones
  //  del fabricante.
  frame[3] = 0xFC;

  do {
    //uart.write(frame,frame[1]);
    for (uint8_t i = 0; i <= frame[1]-1; i++){
      uart.write(frame[i]);
      delay(1);
      #ifdef _DEBUBG_TX
        debug.write(frame[i]);
      #endif
    }
    intentos++;
  } while (checkACK() == NACK && intentos <= MAX_INTENTOS);

  return;
}

 /******************************************************************************
  * Función : MCPeraseeprom
  * @brief  : Borra la memoria eeprom del MCP39F511X.
  * @param  : none
  * @return : none
  *******************************************************************************/
void MCPeraseeprom(){

  uint8_t frame[4];
  uint8_t intentos = 0;   // Varible que indica el número de veces que se realiza
                          // la escritura.

  //  Se registra la trama de datos que se desea enviar.
  frame[0] = MCP_ID;
  frame[1] = 0x04; // El número de bytes para esta funcion es fijo.
  frame[2] = MCP_ERASE_EEPROM;
  // El codigo de verificacion se calcula segun las indicaciones
  //  del fabricante.
  frame[3] = 0xF8;

  do {
    //uart.write(frame,frame[1]);
    for (uint8_t i = 0; i <= frame[1]-1; i++){
      uart.write(frame[i]);
      delay(1);
      #ifdef _DEBUBG_TX
        debug.write(frame[i]);
      #endif
    }
    intentos++;
  } while (checkACK() == NACK && intentos <= MAX_INTENTOS);

  return;
}

/******************************************************************************
 * Función : MCPautogain
 * @brief  : Indica al MCP39F511X realizar un autocalibrado de la ganancia de la magnitud indicado.
 * @param  : parameter, magnitud sobre la cual se realizará el autocalibrado (Corriente, Tensión o Potencia activa)
 * @return : none
 *******************************************************************************/
void MCPautogain(uint8_t parameter){

  uint8_t frame[4];
  uint8_t intentos = 0;   // Varible que indica el número de veces que se realiza
                          // la escritura.

  //  Se registra la trama de datos que se desea enviar.
  frame[0] = MCP_ID;
  frame[1] = 0x04; // El número de bytes para esta funcion es fijo.
  frame[2] = parameter;

  frame[frame[1]-1] = Getchecksum(frame);

  do {
  //  uart.write(frame,frame[1]);
    for (uint8_t i = 0; i <= frame[1]-1; i++){
      uart.write(frame[i]);
      delay(1);
      #ifdef _DEBUBG_TX
        debug.write(frame[i]);
      #endif
    }
    intentos++;
  } while (checkACK() == NACK && intentos <= MAX_INTENTOS);

  return;
}

/******************************************************************************
 * Función : isReadyMCP39F511N
 * @brief  : Comprueba si el MCP39F511X se encuentra conectado y la comunicación
 *            se establece correctamente.
 * @param  : none
 * @return : none
 *******************************************************************************/
bool isReadyMCP39F511N(){

  uart.write(!MCP_ID);

  if (checkACK() == NACK)
    return (uart.read() == ID_BYTE);
  else
    return false;

}

/******************************************************************************
 * Función : MCPwriteeprom(*data)
 * @brief  : Guarda en la memoria eeprom del MCP39F511X los datos indicados.
 * @param  : data, puntero al array de datos que se desea guardar en la EEPRom del MCP39F511X
 * @return : none
 *******************************************************************************/
 /* PENDIENTE DE IMPLEMENTAR */


/******************************************************************************
 * Función : MCPreadeprom
 * @brief  : Lee de la memoria eeprom del MCP39F511X
 * @param  : data, puntero al array de datos donde se guardarán la información leída
 * @return : none
 *******************************************************************************/
 /* PENDIENTE DE IMPLEMENTAR */

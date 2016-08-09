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
 #define MCP_ID 0xA5
 #define ACK 0x06
 #define NACK 0x15
 #define CSFAIL 0x51
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
 enum REGITRO{
      VOLTAGE_RMS = 0x0006,
      LINE_FREQUENCY = 0x0008,
      ANALOG_INPUT_VOLTAGE = 0x000A,
      POWER_FACTOR = 0x000C,
      CURRENT_RMS = 0x000E,
     // Ultimo registro
     LAST_REGISTER

 };

 enum BITS_REGITRO{
      BVOLTAGE_RMS = 0X02,
      BLINE_FREQUENCY = 0x02,
      BANALOG_INPUT_VOLTAGE = 0x02,
      BPOWER_FACTOR = 0x02,
      BCURRENT_RMS = 0x02,
      // Ultimo registro
      BLAST_REGISTER
    };

/* Prototipo de Funciones */
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

/* Variables */


/*

 MCP39F511X.c - MCP uart comunication function

 */

/* void error(uint8_t code)
Funcion que avisa e indica del error ocurrido */
void error(uint8_t code){

    String message = "No se ha producido ningun error.";

    digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));

    switch (code){
      case NOCOMUNICACION: /*debug.println(*/message="Error, no hay comunicacion. No se recibe respuesta por parte del receptor.";                  break;
      case CHECKSUMERR:    /*debug.println(*/message="Error, la informacion recibida no es correcta. (Fallo checksum)";                             break;
      case ENACK:          /*debug.println(*/message="Error, se ha recibido un NACK en la confirmación de la respuesta. (Posible comando incorrecto)"; break;
      case ECSFAIL:        /*debug.println(*/message="Error, se ha recibido un CSFAIL en la confirmación de la respuesta. (Comprobar checksum)"; break;
    }

#ifdef _DEBUG_ERROR
  //  cliente.write(message);
#endif
}

/* void MCPwrite(register,num_bytes,dato)
  Permite comprobar si se recibe confirmación de recepción de los
  datos enviados al MCP */
uint8_t checkACK(){

  uint8_t check;

  // Se espera a byte de confirmación de recepción.
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

    return NACK;
  }

  return check;
}

/* uint8_t Getchecksum(*frame)
  Devuelve el código de verifación de trama según las indicaciones
  del fabricante */
uint8_t Getchecksum(uint8_t *frame){

  uint16_t checksum = 0;

  for (uint8_t i = 0; i <= frame[1]-2; i++){
    checksum += frame[i];
  }
  return (uint8_t) checksum;

}

/* void MCPwrite(reg,num_bytes,dato)
  Permite escribir a partir del registro indicado los bytes indicados. */
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

/* void MCPwrite(reg,num_bytes,dato)
  Realiza la escritura indicda en el mcp y asegura que esta se ha realizado
  adecuadamente */
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

/* uint32_t mcp_read(reg,num_bytes)
  Permite leer a partir del registro indicado el
  número de bytes solicitados. */
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
  for (uint8_t i = (frame[1]-3); i >= 1; i--){
    //   [2+(frame[1]-3)-i] -- El -3 es debio a que ya se han leído los 2 primeros valores recibidos.
    frame[frame[1]-1-i] = uart.read();
    readregister = readregister | (frame[frame[1]-1-i] << (i-1)*8); // (i-1) El -1 es debido al checksum
    delay(1);
#ifdef _DEBUBG_RX
    debug.write(readregister);
#endif
  }

  if (Getchecksum(frame) == uart.read())
    return readregister;
#ifdef _DEBUG_ERROR
  else
    error(CHECKSUMERR);
#endif

  return -1;
}

/* void MCPwrite(reg,num_bytes,dato)
  Realiza la lectura indicda en el mcp y asegura que esta se ha realizado
  adecuadamente */
uint32_t MCPread(uint16_t reg, uint8_t num_bytes){

  uint32_t check = NACK;     // Variable de comprobaciación para saber si la escrtura
                          // se ha realizado correctamente.
  uint8_t intentos = 0;   // Varible que indica el número de veces que se realiza
                          // la escritura.

  do {
    check = _MCPread(&reg,&num_bytes);
    intentos++;
  } while (check == NACK && intentos <= MAX_INTENTOS);

  return check;

}

/**
 * Modifica la dirección del puntero del MCP39F511X al registro indicado.
 * @param reg direccion de 16 bit a la cual se desea apuntar el puntero del MCP
 **/
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
    uart.write(frame,frame[1]);
    intentos++;
  } while (checkACK() == NACK && intentos <= MAX_INTENTOS);

  return;
}

/**
 * Guarda las modoficaciones realizadas en el MCP39F511X en la memoria flash
 **/
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
    uart.write(frame,frame[1]);
    intentos++;
  } while (checkACK() == NACK && intentos <= MAX_INTENTOS);

  return;
}

/**
 * Borra la memoria eeprom del MCP39F511X
 **/
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
    uart.write(frame,frame[1]);
    intentos++;
  } while (checkACK() == NACK && intentos <= MAX_INTENTOS);

  return;
}

/**
 * Indica al MCP39F511X realizar un autocalibrado de la ganacia,
 * ganancia reactiva o frecuencia
**/
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
    uart.write(frame,frame[1]);
    intentos++;
  } while (checkACK() == NACK && intentos <= MAX_INTENTOS);

  return;
}

/* void MCPwriteeprom(*data)
  Guarda en la memoria eeprom del MCP39F511X los datos indicados*/

/* void MCPreadeprom(*data)
  Lee de la memoria eeprom del MCP39F511X */

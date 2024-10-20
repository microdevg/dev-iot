#ifndef QMAX_MODBUS_H
#define QMAX_MODBUS_H


#include "modbus_params.h"  // for modbus parameters structures
#include "mbcontroller.h"

/**
 * @brief Inicio una rutina modbus simple. 
 * Consiste en configurar y hacer una peticion modbus
 * 
 * @param cfg 
 * @param slave_address 
 */
void modbus_init(mb_communication_info_t* cfg,uint8_t slave_address);

/**
 * @brief A partir de un json genero una configuracion modbus valida,
 * Ademas como valor de retorno devuelvo la direccion del exclavo modbus
 * 
 * @param json Archivos json, vendra por mqtt o algo asi
 * @param comm objeto de configuracion vacio.
 * @return uint8_t Si fall -1 , si todo ok devuelvo direccion slave modbus
 */
uint8_t modbus_json_config(char* json ,mb_communication_info_t* comm);


#endif
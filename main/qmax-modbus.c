#include "qmax-modbus.h"





#include "string.h"
#include "esp_log.h"




#include <stdlib.h>

#include "jsmn.h"
#include <string.h>


//#include "sdkconfig.h"

#define CONFIG_MB_UART_PORT_TWO 1
#define CONFIG_MB_UART_PORT_NUM 2
#define CONFIG_MB_UART_BAUD_RATE 115200
#define CONFIG_MB_UART_RXD 22
#define CONFIG_MB_UART_TXD 23
#define CONFIG_MB_UART_RTS 18
//#define CONFIG_MB_COMM_MODE_RTU 1


#define MB_PORT_NUM     (CONFIG_MB_UART_PORT_NUM)   // Number of UART port used for Modbus connection
#define MB_DEV_SPEED    (CONFIG_MB_UART_BAUD_RATE)  // The communication speed of the UART

// Note: Some pins on target chip cannot be assigned for UART communication.
// See UART documentation for selected board and target to configure pins using Kconfig.

// The number of parameters that intended to be used in the particular control process
#define MASTER_MAX_CIDS num_device_parameters

// Number of reading of parameters from slave
#define MASTER_MAX_RETRY 3

// Timeout to update cid over Modbus
#define UPDATE_CIDS_TIMEOUT_MS          (500)
#define UPDATE_CIDS_TIMEOUT_TICS        (UPDATE_CIDS_TIMEOUT_MS / portTICK_PERIOD_MS)

// Timeout between polls
#define POLL_TIMEOUT_MS                 (1)
#define POLL_TIMEOUT_TICS               (POLL_TIMEOUT_MS / portTICK_PERIOD_MS)

// The macro to get offset for parameter in the appropriate structure
#define HOLD_OFFSET(field) ((uint16_t)(offsetof(holding_reg_params_t, field) + 1))
#define INPUT_OFFSET(field) ((uint16_t)(offsetof(input_reg_params_t, field) + 1))
#define COIL_OFFSET(field) ((uint16_t)(offsetof(coil_reg_params_t, field) + 1))
// Discrete offset macro
#define DISCR_OFFSET(field) ((uint16_t)(offsetof(discrete_reg_params_t, field) + 1))

#define STR(fieldname) ((const char*)( fieldname ))
// Options can be used as bit masks or parameter limits
#define OPTS(min_val, max_val, step_val) { .opt1 = min_val, .opt2 = max_val, .opt3 = step_val }

static const char *TAG = "MASTER_TEST";




// Enumeration of modbus device addresses accessed by master device
enum {
    MB_DEVICE_ADDR1 = 1 // Only one slave device used for the test (add other slave addresses here)
};

// Enumeration of all supported CIDs for device (used in parameter definition table)
enum {
    CID_INP_DATA_0 = 0,
    CID_HOLD_DATA_0,
    CID_INP_DATA_1,
    CID_HOLD_DATA_1,
    CID_INP_DATA_2,
    CID_HOLD_DATA_2,
    CID_HOLD_TEST_REG,
    CID_RELAY_P1,
    CID_RELAY_P2,
    CID_DISCR_P1,
    CID_COUNT
};




// Example Data (Object) Dictionary for Modbus parameters:
// The CID field in the table must be unique.
// Modbus Slave Addr field defines slave address of the device with correspond parameter.
// Modbus Reg Type - Type of Modbus register area (Holding register, Input Register and such).
// Reg Start field defines the start Modbus register number and Reg Size defines the number of registers for the characteristic accordingly.
// The Instance Offset defines offset in the appropriate parameter structure that will be used as instance to save parameter value.
// Data Type, Data Size specify type of the characteristic and its data size.
// Parameter Options field specifies the options that can be used to process parameter value (limits or masks).
// Access Mode - can be used to implement custom options for processing of characteristic (Read/Write restrictions, factory mode values and etc).
const mb_parameter_descriptor_t device_parameters[] = {
    // { CID, Param Name, Units, Modbus Slave Addr, Modbus Reg Type, Reg Start, Reg Size, Instance Offset, Data Type, Data Size, Parameter Options, Access Mode}
    { CID_INP_DATA_0, STR("Data_channel_0"), STR("Volts"), MB_DEVICE_ADDR1, MB_PARAM_INPUT, 0, 2,
            INPUT_OFFSET(input_data0), PARAM_TYPE_FLOAT, 4, OPTS( -10, 10, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_HOLD_DATA_0, STR("Humidity_1"), STR("%rH"), MB_DEVICE_ADDR1, MB_PARAM_HOLDING, 0, 2,
            HOLD_OFFSET(holding_data0), PARAM_TYPE_FLOAT, 4, OPTS( 0, 100, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_INP_DATA_1, STR("Temperature_1"), STR("C"), MB_DEVICE_ADDR1, MB_PARAM_INPUT, 2, 2,
            INPUT_OFFSET(input_data1), PARAM_TYPE_FLOAT, 4, OPTS( -40, 100, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_HOLD_DATA_1, STR("Humidity_2"), STR("%rH"), MB_DEVICE_ADDR1, MB_PARAM_HOLDING, 2, 2,
            HOLD_OFFSET(holding_data1), PARAM_TYPE_FLOAT, 4, OPTS( 0, 100, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_INP_DATA_2, STR("Temperature_2"), STR("C"), MB_DEVICE_ADDR1, MB_PARAM_INPUT, 4, 2,
            INPUT_OFFSET(input_data2), PARAM_TYPE_FLOAT, 4, OPTS( -40, 100, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_HOLD_DATA_2, STR("Humidity_3"), STR("%rH"), MB_DEVICE_ADDR1, MB_PARAM_HOLDING, 4, 2,
            HOLD_OFFSET(holding_data2), PARAM_TYPE_FLOAT, 4, OPTS( 0, 100, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_HOLD_TEST_REG, STR("Test_regs"), STR("__"), MB_DEVICE_ADDR1, MB_PARAM_HOLDING, 10, 58,
            HOLD_OFFSET(test_regs), PARAM_TYPE_ASCII, 116, OPTS( 0, 100, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_RELAY_P1, STR("RelayP1"), STR("on/off"), MB_DEVICE_ADDR1, MB_PARAM_COIL, 2, 6,
            COIL_OFFSET(coils_port0), PARAM_TYPE_U8, 1, OPTS( 0xAA, 0x15, 0 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_RELAY_P2, STR("RelayP2"), STR("on/off"), MB_DEVICE_ADDR1, MB_PARAM_COIL, 10, 6,
            COIL_OFFSET(coils_port1), PARAM_TYPE_U8, 1, OPTS( 0x55, 0x2A, 0 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_DISCR_P1, STR("DiscreteInpP1"), STR("on/off"), MB_DEVICE_ADDR1, MB_PARAM_DISCRETE, 2, 7,
            DISCR_OFFSET(discrete_input_port1), PARAM_TYPE_U8, 1, OPTS( 0xAA, 0x15, 0 ), PAR_PERMS_READ_WRITE_TRIGGER }
};
// { CID_DISCR_P1, STR("DiscreteInpP1"), STR("on/off"), MB_DEVICE_ADDR1, MB_PARAM_DISCRETE, 2, 7,
//   DISCR_OFFSET(discrete_input_port1), PARAM_TYPE_U8, 1, OPTS( 0xAA, 0x15, 0 ), PAR_PERMS_READ_WRITE_TRIGGER }
//
// Calculate number of parameters in the table
const uint16_t num_device_parameters = (sizeof(device_parameters)/sizeof(device_parameters[0]));




// Modbus master initialization
static esp_err_t master_init(mb_communication_info_t* comm)
{

    void* master_handler = NULL;

    esp_err_t err = mbc_master_init(MB_PORT_SERIAL_MASTER, &master_handler);
    MB_RETURN_ON_FALSE((master_handler != NULL), ESP_ERR_INVALID_STATE, TAG,
                                "mb controller initialization fail.");
    MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG,
                            "mb controller initialization fail, returns(0x%x).", (int)err);

    err = mbc_master_setup((void*)comm);
    MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG,
                            "mb controller setup fail, returns(0x%x).", (int)err);

    err = uart_set_pin(MB_PORT_NUM, 22, 23, 18, UART_PIN_NO_CHANGE);
    MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG,
        "mb serial set pin failure, uart_set_pin() returned (0x%x).", (int)err);

    err = mbc_master_start();
    MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG,
                            "mb controller start fail, returned (0x%x).", (int)err);


     err = uart_set_mode(MB_PORT_NUM,     UART_MODE_UART );
    MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG,
            "mb serial set mode failure, uart_set_mode() returned (0x%x).", (int)err);

    vTaskDelay(5);
    err = mbc_master_set_descriptor(&device_parameters[0], num_device_parameters);
    MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG,
                                "mb controller set descriptor fail, returns(0x%x).", (int)err);
    ESP_LOGI(TAG, "Modbus master stack initialized...");
    return err;
}



void modbus_init(mb_communication_info_t* cfg,uint8_t slave_address)
{
     // Initialization of device peripheral and objects
   printf("Iniciamos modbus con configuracion extraida de json file\n");
    ESP_ERROR_CHECK(master_init(cfg));
    vTaskDelay(10);

   // master_operation_func(NULL);

#define SLAVE_ADRRESS          7
#define START_REGISTER         2
#define N_DEVICE_REGISTER      8
#define FUNC_READ_REGISTERS   3 // Codigo de funcion modbus, leer manual
    
// Peticion simple
    mb_param_request_t req1 = {
        .slave_addr = slave_address,
        .command = FUNC_READ_REGISTERS ,      // leer registro
        .reg_start = START_REGISTER,      // Direccion inicial del registro
        .reg_size = N_DEVICE_REGISTER       // Cantidad de registro
    };




uint16_t devices_registers[N_DEVICE_REGISTER]={0};

  mbc_master_send_request(&req1,devices_registers);

    vTaskDelay(10);

  printf("data 1: %d\n",devices_registers[0]);
  printf("data 2: %d\n",devices_registers[1]);
  printf("data 3: %d\n",devices_registers[2]);
  printf("data 4: %d\n",devices_registers[3]);
  printf("data 5: %d\n",devices_registers[4]);
  printf("data 6: %d\n",devices_registers[5]);
  printf("data 7: %d\n",devices_registers[6]);
  printf("data 8: %d\n",devices_registers[7]);

  printf("Finalizo exitosamente\n");

}







#define NUM_PARAMETERS                  8
#define MODBUS_PARAM_TYPE           "TYPE"
#define MODBUS_PARAM_BAUDIOS        "BAUDRATE"
#define MODBUS_PARAM_PARIRY         "PARITY"
#define MODBUS_PARAM_ADDRS          "ADDRSLAVE"


/**
 * @brief Ejemplo de json de configuracion para mb serial rtu
 *   {  "TYPE":0,               // RTU == 0 , ASCII == 1 , TCPIP == 2
 *      "BAUDRATE": 115200,
 *      "PARITY": 0 ,          // NO == 0 , SI == 1
 *      "ADDRSLAVE": 7 
 *      }
 */


#define JSON_FORMAT_ERROR     (-1)
#define JSON_PARSE_SUCCESS      (1)
#define JSON_PARSE_TYPE_RTU     0
#define JSON_PARSE_TYPE_ASCII   1
#define JSON_PARSE_TYPE_TCPIP   2
#define JSON_PARSE_PARITY_NONE  0
#define JSON_PARSE_PARITY_EVEN  1
#define JSON_PARSE_PARITY_ODD   2



// Macros para simplificar el chequeo de tipo y comparación
// Macro generalizada para verificar token y tipo
#define PARSE_JSON_BUFFER_LEN   50
#define GET_POUNTER_TOKEN_STRING(tokens,idx)      &json[tokens[idx].start]
#define CHECK_JSON_TOKEN_ELEMENT(tokens, idx, json_type, expected_obj) \
    if ((tokens[idx].type != json_type) || (strncmp( &json[tokens[idx].start], expected_obj,(tokens[idx].end - tokens[idx].start)) != 0)) \
        return JSON_FORMAT_ERROR;











 




uint8_t modbus_json_config(char* json ,mb_communication_info_t* comm)
{

comm->port = MB_PORT_NUM; // El puerto siempre es el mismo
char buffer[PARSE_JSON_BUFFER_LEN] = {0};

jsmntok_t tokens[NUM_PARAMETERS+1]; // El primer no es valido.
jsmn_parser parser;
jsmn_init(&parser);
jsmn_parse(&parser, json, strlen(json), tokens, NUM_PARAMETERS+1);


CHECK_JSON_TOKEN_ELEMENT(tokens,1,JSMN_STRING,MODBUS_PARAM_TYPE);
CHECK_JSON_TOKEN_ELEMENT(tokens,3,JSMN_STRING,MODBUS_PARAM_BAUDIOS);
CHECK_JSON_TOKEN_ELEMENT(tokens,5,JSMN_STRING,MODBUS_PARAM_PARIRY);
CHECK_JSON_TOKEN_ELEMENT(tokens,7,JSMN_STRING,MODBUS_PARAM_ADDRS);

//char * type =             GET_POUNTER_TOKEN_STRING(tokens,1);
char * type_val =         GET_POUNTER_TOKEN_STRING(tokens,2);
//char * baud =             GET_POUNTER_TOKEN_STRING(tokens,3);
char * baud_val =         GET_POUNTER_TOKEN_STRING(tokens,4);

//char * parity =           GET_POUNTER_TOKEN_STRING(tokens,5);
char * parity_val =       GET_POUNTER_TOKEN_STRING(tokens,6);
//char * addrs =            GET_POUNTER_TOKEN_STRING(tokens,7);
char * addrs_val =        GET_POUNTER_TOKEN_STRING(tokens,8);

//int    type_len =       tokens[1].end - tokens[1].start;
int    type_val_len =   tokens[2].end - tokens[2].start;
//int    baud_len =       tokens[3].end - tokens[3].start;
int    baud_val_len =   tokens[4].end - tokens[4].start;
//int    parity_len =     tokens[5].end - tokens[5].start;
int    parity_val_len = tokens[6].end - tokens[6].start;
//int    addrs_len =      tokens[7].end - tokens[7].start;
int    addrs_val_len =  tokens[8].end - tokens[8].start;




//printf("type:%.*s\n",type_len,type);
//printf("type:%.*s\n",type_val_len,type_val);
//printf("baud:%.*s\n",baud_len,baud);
//printf("baud:%.*s\n",baud_val_len,baud_val);
//printf("parity:%.*s\n",parity_len,parity);
//printf("parity:%.*s\n",parity_val_len,parity_val);
//printf("addrs:%.*s\n",addrs_len,addrs);
//printf("addrs:%.*s\n",addrs_val_len,addrs_val);


strncpy(buffer,type_val,type_val_len);
buffer[type_val_len]=0;
int modbus_type  = atoi(buffer); // Convierte la cadena a un entero

switch (modbus_type)
  {
        case JSON_PARSE_TYPE_RTU:
                printf("modbus type: RTU\n");
                comm->mode = MB_MODE_RTU;
                break;
        case JSON_PARSE_TYPE_ASCII:
                printf("modbus type: ASCII\n");
                comm->mode = MB_MODE_ASCII;
                break;
        case JSON_PARSE_TYPE_TCPIP:
                printf("modbus type: TCP/IP[AUN NO IMPLEMENTADO]\n");
                comm->mode = MB_MODE_TCP;
                return JSON_FORMAT_ERROR;
                break;
        default:
                return JSON_FORMAT_ERROR;
                break;
  }

     strncpy(buffer,baud_val,baud_val_len);
     buffer[baud_val_len]=0;
     int _baudios  = atoi(buffer); // Convierte la cadena a un entero
     printf("baudios:%d\n",_baudios);

     if( _baudios > 0 && _baudios <= 115200) comm->baudrate = _baudios;
     strncpy(buffer,parity_val,parity_val_len);
     buffer[parity_val_len]=0;
     int _parity  = atoi(buffer); // Convierte la cadena a un entero

     switch (_parity)
  {
        case JSON_PARSE_PARITY_NONE:
                printf("Sin paridad\n");
                comm->parity = MB_PARITY_NONE;
                break;
        case     JSON_PARSE_PARITY_EVEN   :

                printf("Paridad par\n");
                comm->parity = UART_PARITY_EVEN;
                break;
        case JSON_PARSE_PARITY_ODD:
                printf("Paridad impar\n");
                comm->parity = UART_PARITY_ODD;        
                break;
        default:
                return JSON_FORMAT_ERROR;
                break;
  }
     
     strncpy(buffer,addrs_val,addrs_val_len);
     buffer[addrs_val_len]=0;
     int _addrss  = atoi(buffer); // Convierte la cadena a un entero
     printf("adress slave:%d\n",_addrss);

        return _addrss; // OK

}

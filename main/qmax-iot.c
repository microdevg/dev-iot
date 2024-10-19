/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "display.h"
#include "qmax-modbus.h"


// Probemos parseo de json
#include "jsmn.h"
#include <string.h>


const char cfg_mb_json[]="{ \"TIPO_MODBUS\" : \"RTU\", \"BAUDIOS\" : 115200 , \"PARIDAD\": \"NO\"}";  // Contiene 4 tokens





void app_main(void)
{
 
    //display_init(  "WiFi CONECTADO", "Modbus RTU", " Probando modbus");
   // printf("Prueba Modbus, request simple\n");
   // modbus_init();
    jsmn_parser parser;
    #define NUM_TOKENS      5
    jsmntok_t tokens[NUM_TOKENS];

    jsmn_init(&parser);

    // js - pointer to JSON string
    // tokens - an array of tokens available
    // 10 - number of tokens available
    printf("Quiero parsear \n%s\n",cfg_mb_json);
    jsmn_parse(&parser, cfg_mb_json, strlen(cfg_mb_json), tokens, NUM_TOKENS);


    for(int i = 0; i< NUM_TOKENS ; i++)
    {
       // printf("token: %d\n",i);
        switch (tokens[i].type)
        {
        case JSMN_UNDEFINED:
            printf("es indefinido\n");
            break;
        
        case JSMN_ARRAY: 
            printf("define un array de elementos\n");
            break;
        
        case JSMN_STRING:
            printf("define un string\n");
            break;

        case JSMN_PRIMITIVE:
            printf("define un primitivo\n");
            break;
        
        default:
            break;
        }   
        printf("->%.*s\n",(tokens[i].end-tokens[i].start),&cfg_mb_json[tokens[i].start]);

    }


   printf("Probando libreria jsmn para manejar json\n");


   
 while(1){
    
  


    vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
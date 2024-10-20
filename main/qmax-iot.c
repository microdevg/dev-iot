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


   // RTU == 0 , ASCII == 1 , TCPIP == 2

char json[] = "{\"TYPE\":1,\"BAUDRATE\": 19200,\"PARITY\": 0 ,\"ADDRSLAVE\": 12}";



void app_main(void)
{
 
   printf("parseo de :%s\n",json);

  mb_communication_info_t comm = {0};
   // Si sale todo bien retorna la direccion del esclavo, sino -1 por error
  int slave =  modbus_json_config( json,&comm);

if(slave != -1){
   printf("comunicate con slave:%d\n",slave);
   modbus_init(&comm,slave);
   char buf[30] = {0};
   sprintf(buf,"MBSLAVE:%d",slave);
   char buf2[30] = {0};
   sprintf(buf2,"Baudrate:%ld",comm.baudrate);
   display_init(  "WiFi CONECTADO",buf, buf2);
}
else{
   printf("Erro al configurar\n");
}



   
 while(1){
    
  


    vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
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




void app_main(void)
{
 
// Inicio la pantalla
    display_init(  "WiFi CONECTADO", "Modbus RTU", " Probando modbus");
    printf("Prueba Modbus, request simple\n");

    modbus_init();
   
 while(1){
    
  


    vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
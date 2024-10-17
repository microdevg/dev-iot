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


/**
 * Ejemplo de uso de la pantalla con una 
 * secuencia de evento que escribe la pantalla
 */

void app_main(void)
{
    printf(" Oled test example\n");   
    int counter = 0;
    char buffer[50]={0};
    modbus_init();
    display_init(  NULL, "Modbus RTU init", NULL);   
 while(1){
    sprintf(buffer,"counter: %d\n",counter++);
    printf(buffer);


    vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
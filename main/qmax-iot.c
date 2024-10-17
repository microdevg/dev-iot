/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "display.h"



/**
 * Ejemplo de uso de la pantalla con una 
 * secuencia de evento que escribe la pantalla
 */

void app_main(void)
{
    printf(" Oled test example\n");   
    int counter = 0;
    char buffer[50]={0};
    display_init(  NULL, "Modbus RTU init", NULL);   
 while(1){
    sprintf(buffer,"counter: %d\n",counter++);
    printf(buffer);

if(counter < 20)display_send_conn((const char*)buffer);
else     display_send_conn("WiFi Desconectado");

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    if(counter == 10){
        display_send_alarm("Alarma B\n");
    }
    if(counter == 14){
        display_send_modbus("Modbus falled\n");
    }
    if(counter == 25){
        display_send_alarm("Alarma Off");
        display_send_modbus("Modbus Conectado");
    }

    }
}
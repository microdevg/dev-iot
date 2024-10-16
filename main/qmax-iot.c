/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "display.h"





void app_main(void)
{
    printf(" Oled test example\n");   
    int counter = 0;
    display_init(  NULL, "Modbus RTU init", NULL);   
 while(1){



    printf("counter:%d\n",counter++);

    vTaskDelay(1000 / portTICK_PERIOD_MS);


    }
}
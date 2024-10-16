#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>





void Task_main(void* param);




void app_main(void)
{
    static int counter = 0;

    xTaskCreate(&Task_main, "Demo Task", 3000, &counter, 5, NULL);

}

ssd1306_handle_t oled ;

void Task_main(void* param)
{
    int* _counter = (int*)param;
    


    while (true) {
        printf("counter:%d\n",(*_counter));
        (*_counter)++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
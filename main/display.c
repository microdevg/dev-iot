#include "display.h"


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "freertos/queue.h"

#define QUEUE_ELEMENT_NMAX 3


typedef enum{
    DISPLAY_CONN_MESSAGE,
    DISPLAY_MODBUS_MESSAGE,
    DISPLAY_ALARM_MESSAGE
} msg_type_t;

typedef struct {
msg_type_t type;
const char* message;
}display_msg_type_t;


QueueHandle_t display_queue  = NULL;

static const char *TAG = "example";


#include "esp_lcd_panel_vendor.h"

/**
 *  La pantalla tendra 3 elementos
 * 
 *          | Conexion con MQTT| Ejemplo: conectado a WiFi/mqtt      |
 *          | Conexion Modbus|  Ejemplo: dispositivo conectado, etc  |
 *          | Estado de las alarmas| Ejemplo: Sensor de temperatura  |    
 *          
 */



    lv_obj_t *scr = NULL;
    lv_obj_t *conn_label  = NULL;
    lv_obj_t *modbus_label = NULL;
    lv_obj_t *alarm_label = NULL;


static void display_task(void* params);


void display_init(  const char* conn_text, 
                    const char* modbus_text,
                    const char* alarm_text)
{
    i2c_master_bus_handle_t i2c_bus = NULL;
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .i2c_port = I2C_BUS_PORT,
        .sda_io_num = EXAMPLE_PIN_NUM_SDA,
        .scl_io_num = EXAMPLE_PIN_NUM_SCL,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = EXAMPLE_I2C_HW_ADDR,
        .scl_speed_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .control_phase_bytes = 1,               // According to SSD1306 datasheet
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,   // According to SSD1306 datasheet
        .lcd_param_bits = EXAMPLE_LCD_CMD_BITS, // According to SSD1306 datasheet
        .dc_bit_offset = 6,                     // According to SSD1306 datasheet

    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install SSD1306 panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    ESP_LOGI(TAG, "Initialize LVGL");
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES,
        .double_buffer = true,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = true,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        }
    };
    lv_disp_t *disp = lvgl_port_add_disp(&disp_cfg);

    /* Rotation of the screen */
    lv_disp_set_rotation(disp, LV_DISP_ROT_180);
    ESP_LOGI(TAG, "Display LVGL Scroll Text");
    if(lvgl_port_lock(0)) {
        scr = lv_disp_get_scr_act(disp);
        conn_label = lv_label_create(scr); 
        lv_label_set_long_mode(conn_label,LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_width(conn_label,disp->driver->hor_res);
        lv_obj_align(conn_label,     LV_ALIGN_TOP_LEFT, 0, 0);
        lv_label_set_text(conn_label,(conn_text)?conn_text:"Estado de la conexion");

         modbus_label = lv_label_create(scr); 
        //lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR); /* Circular scroll */
        lv_label_set_long_mode(modbus_label,LV_LABEL_LONG_SCROLL_CIRCULAR);
        /* Size of the screen (if you use rotation 90 or 270, please set disp->driver->ver_res) */
        lv_obj_set_width(modbus_label,disp->driver->hor_res);
        lv_obj_align(modbus_label,     LV_ALIGN_TOP_LEFT, 0, 20);
        lv_label_set_text(modbus_label,(modbus_text)?modbus_text:"Estado Modbus conn");

        alarm_label = lv_label_create(scr); 
        //lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR); /* Circular scroll */
        lv_label_set_long_mode(alarm_label,LV_LABEL_LONG_SCROLL_CIRCULAR);
        /* Size of the screen (if you use rotation 90 or 270, please set disp->driver->ver_res) */
        lv_obj_set_width(alarm_label,disp->driver->hor_res);
        lv_obj_align(alarm_label,     LV_ALIGN_TOP_LEFT, 0, 40);
        lv_label_set_text(alarm_label,(alarm_text)?alarm_label:"Estado de Alarma");
        
        
        lvgl_port_unlock();
    }

    xTaskCreate(display_task,"Display task",4000,NULL,10,NULL);

}



void display_send_alarm(const char* msg){
    display_msg_type_t alarm = {0};
    alarm.message  = msg;
    alarm.type = DISPLAY_ALARM_MESSAGE;
    xQueueSend(display_queue,&alarm,1000);
}
void display_send_modbus(const char* msg){
    display_msg_type_t modbus = {0};
    modbus.message  = msg;
    modbus.type = DISPLAY_MODBUS_MESSAGE;
    xQueueSend(display_queue,&modbus,1000);
}

void display_send_conn(const char* msg){

    display_msg_type_t conn = {0};
    conn.message  = msg;
    conn.type = DISPLAY_CONN_MESSAGE;
    xQueueSend(display_queue,&conn,1000);
}




void display_task(void* params){


    display_queue = xQueueCreate(QUEUE_ELEMENT_NMAX,sizeof(display_msg_type_t));

    display_msg_type_t msg = {0};




    while(1){
    
    if (xQueueReceive(display_queue, &msg, (TickType_t) 10000) == pdTRUE) {

        // Procesar el mensaje recibido
         if(lvgl_port_lock(0)) {
            //Editar los elementos

            switch (msg.type)
            {

            case DISPLAY_CONN_MESSAGE:
                printf("conn label: %s\n",msg.message);
                lv_label_set_long_mode(conn_label,LV_LABEL_LONG_SCROLL_CIRCULAR);
                lv_label_set_text(conn_label,msg.message);
                lv_obj_set_width(conn_label,EXAMPLE_LCD_H_RES);
                lv_obj_align(conn_label, LV_ALIGN_TOP_LEFT, 0, 0);

                break;    

            case DISPLAY_MODBUS_MESSAGE:
                printf("Modbus message\n");
                /* Size of the screen (if you use rotation 90 or 270, please set disp->driver->ver_res) */
                lv_obj_align(modbus_label, LV_ALIGN_TOP_LEFT, 0, 20);
                lv_label_set_text(modbus_label,msg.message);
                lv_obj_set_width(modbus_label,EXAMPLE_LCD_H_RES);

                break;

            case DISPLAY_ALARM_MESSAGE:
                printf("Alarma message\n");
                lv_label_set_text(alarm_label,msg.message);
                lv_obj_set_width(alarm_label,EXAMPLE_LCD_H_RES);
                lv_obj_align(alarm_label, LV_ALIGN_TOP_LEFT, 0, 40);

                break;

          
            default:
                break;
            }
           // Refrescar pantalla
            lv_task_handler();
            lvgl_port_unlock(); 
         }

       
    }


   
    
    vTaskDelay(100 / portTICK_PERIOD_MS);


    }
}
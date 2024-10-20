# QMAX-IOT

QMAX-IOT es un proyecto diseñado para facilitar la comunicación con equipos compatibles mediante los protocolos Modbus RTU y Modbus TCP, permitiendo la lectura de sus mapas de memoria y el envío de datos a un servidor a través del protocolo MQTT. Este sistema está pensado para integrarse en entornos de Internet de las Cosas (IoT), donde el monitoreo y control remoto de dispositivos industriales es clave.

El proyecto está basado en un **ESP32** y utiliza el entorno de desarrollo de **Espressif IDF** (Espressif IoT Development Framework, IDF.py) para su programación.

## Características principales

- **Compatibilidad Modbus RTU y TCP**: QMAX-IOT soporta ambos protocolos para interactuar con dispositivos Modbus.
- **Lectura del mapa de memoria**: Permite leer registros de los equipos Modbus (coils, discrete inputs, holding registers, input registers).
- **Integración MQTT**: Los datos recolectados son enviados a un servidor mediante MQTT, un protocolo ligero ampliamente usado en soluciones IoT.
- **Configuración flexible**: Posibilidad de configurar las direcciones Modbus, intervalos de lectura, y parámetros de conexión MQTT.

## Requisitos

Para ejecutar QMAX-IOT, asegúrate de cumplir con los siguientes requisitos:

- **Hardware**: 
  - **ESP32** como dispositivo principal.
  - Dispositivo compatible con Modbus RTU/TCP (Ej. PLC, sensor, actuador, etc.)
  - Conexión serie o Ethernet para Modbus RTU/TCP.

- **Software**:
  - **Espressif IoT Development Framework (IDF.py)** 



### Display Oled (I2C)
La configuracion de conexion del display esta en el archivo display.h
```c
main/display.h

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ    (400 * 1000)
#define EXAMPLE_PIN_NUM_SDA           3
#define EXAMPLE_PIN_NUM_SCL           4

```

## Placa de desarrollo utilizada:

![img](./imgs/esp32_pinout.jpg)

#### Display funcionando



![img](./imgs/display_working.gif)


#### Prueba modbus

Prueba sencilla, leo 8 registros consecutivos (16bits). La peticion MODBUS es la siguiente
```c
#define SLAVE_ADRRESS          7
#define START_REGISTER         2
#define N_DEVICE_REGISTER      8
#define FUNC_READ_REGISTERS   3 // Codigo de funcion modbus, leer manual
    
// Peticion simple
    mb_param_request_t req1 = {
        .slave_addr = SLAVE_ADRRESS,
        .command = FUNC_READ_REGISTERS ,      // leer registro
        .reg_start = START_REGISTER,      // Direccion inicial del registro
        .reg_size = N_DEVICE_REGISTER       // Cantidad de registro
    };

```

#### Simulador: comunicacion mediante puerto Serial

![img](./imgs/simulator_slave_modbus.jpg)

#### Simulador: mapa de memoria simulado
![img](./imgs/mapa_memoria.jpg)


#### Terminal

![img](./imgs/terminal_esp32.jpg)

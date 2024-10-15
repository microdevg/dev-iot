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
  - **Espressif IoT Development Framework (IDF.py)** para programar el ESP32.
  - Python 3.x (para herramientas adicionales)
  - Librerías:
    - `pymodbus` (para la comunicación Modbus)
    - `paho-mqtt` (para el envío de datos a través de MQTT)
  
  Puedes instalar las dependencias ejecutando:
  ```bash
  pip install pymodbus paho-mqtt

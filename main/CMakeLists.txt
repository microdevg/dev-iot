#set(PROJECT_NAME "modbus_master")


#idf_component_register(SRCS "display.c" "qmax-iot.c" "qmax-modbus.c"
#                    INCLUDE_DIRS ".")

set(PROJECT_NAME "modbus_master")

# Añadir archivos de mb_example_common a los archivos fuente
idf_component_register(
    SRCS "display.c" "qmax-iot.c" "qmax-modbus.c" 
         "${IDF_PATH}/examples/protocols/modbus/mb_example_common/modbus_params.c"

    INCLUDE_DIRS "." 
                 "${IDF_PATH}/examples/protocols/modbus/mb_example_common/include"
)

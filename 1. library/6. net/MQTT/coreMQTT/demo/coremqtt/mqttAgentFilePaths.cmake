# This file is to add source files and include directories
# into variables so that it can be reused from different repositories
# in their Cmake based build system by including this file.
#
# Files specific to the repository such as test runner, platform tests
# are not added to the variables.

include(${CMAKE_CURRENT_LIST_DIR}/source/dependency/coremqtt/mqttFilePaths.cmake)

# MQTT Agent library Public Include directories.
set(MQTT_AGENT_INCLUDE_PUBLIC_DIRS
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/source/include"
    "${CMAKE_CURRENT_LIST_DIR}/ports/mqtt_agent_interface"
    "${CMAKE_CURRENT_LIST_DIR}/ports/subscription_manager")


# MQTT Agent library source files.
set(MQTT_AGENT_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/source/core_mqtt_agent.c"
    "${CMAKE_CURRENT_LIST_DIR}/source/core_mqtt_agent_command_functions.c")

# MQTT Agent library source files.
set(MQTT_AGENT_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/ports/mqtt_agent_interface/freertos_agent_message.c"
    "${CMAKE_CURRENT_LIST_DIR}/ports/mqtt_agent_interface/freertos_command_pool.c"
    "${CMAKE_CURRENT_LIST_DIR}/ports/subscription_manager/subscription_manager.c")

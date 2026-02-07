#include "bsp_onewire.h"

#include "gd32f30x_gpio.h"
#include "main.h"  // For delay implementation if needed

/* Port specific data structure */
typedef struct {
    uint32_t gpio;
    uint32_t pin;
} onewire_port_data_t;

/* Static instance for the default bus */
static onewire_port_data_t default_port_data;
static onewire_bus_t default_bus;

/* Hardware Operations Implementation */

static void onewire_gpio_set_pin(void *arg, uint8_t level) {
    onewire_port_data_t *port = (onewire_port_data_t *) arg;
    if (level) {
        gpio_bit_set(port->gpio, port->pin);
    } else {
        gpio_bit_reset(port->gpio, port->pin);
    }
}

static uint8_t onewire_gpio_get_pin(void *arg) {
    onewire_port_data_t *port = (onewire_port_data_t *) arg;
    return gpio_input_bit_get(port->gpio, port->pin);
}

/* Operations structure */
static const onewire_ops_t default_ops = {
    .set_pin = onewire_gpio_set_pin,
    .get_pin = onewire_gpio_get_pin,
};

/* Initialization function */
onewire_bus_t *bsp_onewire_init(uint32_t gpio, uint32_t pin) {
    /* Initialize GPIO */
    gpio_init(gpio, GPIO_MODE_OUT_OD, GPIO_OSPEED_50MHZ, pin);
    gpio_bit_set(gpio, pin);  // Default high
    delay_ms(10);             // Wait for stabilization

    /* Store port configuration */
    default_port_data.gpio = gpio;
    default_port_data.pin = pin;

    /* Initialize the bus handle */
    onewire_init(&default_bus, &default_ops, &default_port_data);

    return &default_bus;
}
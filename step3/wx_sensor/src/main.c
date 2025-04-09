
// #include <string.h>
// #include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <kos.h>
#include <kos_utils.h>
#include <i2c_client.h>

// #include "bme280.h"
// #include "bme280_defs.h"
#include "../../drivers/bme280/bme280.h"
#include "../../drivers/bme280/bme280_defs.h"

#define DELAY           1000000
#define BME280_ADDR     BME280_I2C_ADDR_SEC

enum token_slot_index {
    TOKEN_CLIENT_TRANSPORT = 1,
    TOKEN_I2C
};

int8_t bme280_i2c_bus_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr)
{
    kos_i2c_write_synchronous(TOKEN_I2C, BME280_ADDR, &reg_addr, 1);
    kos_i2c_read_synchronous(TOKEN_I2C, BME280_ADDR, data, len);
    return 0;
}

int8_t bme280_i2c_bus_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr)
{
    uint8_t *buf;

    buf = malloc(len + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);

    if (kos_i2c_write_synchronous(TOKEN_I2C, BME280_ADDR, buf, len + 1) < (uint16_t) len) 
    {
        return BME280_E_COMM_FAIL;
    }

    free(buf);

    return BME280_OK;
}

void bme280_delay_msec(uint32_t period_msec, void *intf_ptr)
{
    kos_delay(period_msec);
}

int bme280_assert(int status, char* message) 
{
    if (status != BME280_OK) {
        kos_printf("%s\n", message);
        return 0;
    }
    return 1;
}

void setup_msg_broker(void) {
    kos_status_t status;
  
    kos_cap_t receive_cap = kos_cnode_cap(kos_app_cnode(), KOS_ROOT_RECEIVE);
    kos_cap_set_receive(receive_cap);
  
    status = kos_msg_setup();
    kos_assert_created(status, "failed to setup msg server connection");
}
  
kos_msg_client_t setup_client_transport(void) {
    kos_status_t status;
    kos_cap_t client_cap = kos_cap_reserve();
    kos_token_t token_client_transport;
    kos_msg_client_t msg_client = {0};

    status = kos_msg_client_create(client_cap, TOKEN_CLIENT_TRANSPORT, &msg_client);
    kos_assert_created(status, "failed to create kos_msg_client");

    return msg_client;
}

int main(int argc, char *argv[]) 
{
    kos_status_t status; 

    // Get the i2c port name from the arguments
    // We expect two arguments:
    // 0. app name
    // 1. i2c port name 
    kos_assert_eq(argc, 2, "unexpected number of arguments for weather sensor");
    char* app_name = argv[0];
    char* i2c_port = argv[1];

    kos_printf("Initializing %s\n", app_name);

    // Signal that we are done initializing
    kos_app_ready();

    // Setup the message broker
    kos_cap_t receive_cap = kos_cnode_cap(kos_app_cnode(), KOS_ROOT_RECEIVE);
    kos_cap_set_receive(receive_cap);
  
    status = kos_msg_setup();
    kos_assert_created(status, "failed to setup msg server connection");

    kos_printf("Message broker setup.\n");

    // Create a client transport
    kos_cap_t client_cap = kos_cap_reserve();
    kos_token_t token_client_transport;
    kos_msg_client_t msg_client = {0};

    status = kos_msg_client_create(client_cap, TOKEN_CLIENT_TRANSPORT, &msg_client);
    kos_assert_created(status, "failed to create kos_msg_client");

    kos_printf("Client transport setup.\n");

    /*
    *   I2C SERVER
    */
    status = kos_dir_request(i2c_port, TOKEN_I2C, KOS_MSG_FLAG_SEND_TOKEN, NULL);
    while(status != STATUS_OK) {
        kos_printf("Could not connect to %s. Retrying after 1 s.\n", i2c_port);
        kos_delay(1000000);
        status = kos_dir_request(i2c_port, TOKEN_I2C, KOS_MSG_FLAG_SEND_TOKEN, NULL);
    }
    kos_assert_ok(status, "Problem connecting to i2c service");
  
    kos_printf("I2C connection established.\n");

        /*
    *   BME280 SETUP AND INITIALIZATION
    */
    int8_t bme280_status = BME280_OK;
    
    // Set the i2c interface 
    struct bme280_dev bme280_sensor;
    bme280_sensor.intf     = BME280_I2C_INTF;
    bme280_sensor.read     = bme280_i2c_bus_read;
    bme280_sensor.write    = bme280_i2c_bus_write;
    bme280_sensor.delay_us = bme280_delay_msec;

    // Set sensor settings
    struct bme280_settings settings;
    settings.osr_h  = BME280_OVERSAMPLING_1X;
    settings.osr_p  = BME280_OVERSAMPLING_16X;
    settings.osr_t  = BME280_OVERSAMPLING_2X;
    settings.filter = BME280_FILTER_COEFF_16;

    kos_printf("Initialize the BME280 sensor\n");

    bme280_status = bme280_init(&bme280_sensor);
    bme280_assert(bme280_status, "Failed to initialize the device");

    // Set what to measure and whether to filter
    uint8_t selection = BME280_SEL_OSR_PRESS  \
                        | BME280_SEL_OSR_TEMP \
                        | BME280_SEL_OSR_HUM  \
                        | BME280_SEL_FILTER;

    kos_printf("Set sensor settings\n");

    /*
    *  FIX THIS
    */
    bme280_status = bme280_set_sensor_settings(selection, &settings, &bme280_sensor);
    bme280_assert(bme280_status, "Failed to get sensor settings");

    /* 
    *   BME280 GET DATA
    */
    struct bme280_data sensor_data;
    float temperature, pressure, humidity;
    
    do {

        kos_printf("Set sensor mode\n");

        // Set the mode to forced mode.  This happens  every loop.
        bme280_status = bme280_set_sensor_mode(BME280_POWERMODE_FORCED, &bme280_sensor); 
        bme280_assert(bme280_status, "Failed to set sensor mode");

        // kos_printf("Delay for some reason");
        // bme280_sensor.delay_us(DELAY, bme280_sensor.intf_ptr);

        kos_printf("==============================\n");
        kos_printf("Get sensor data\n");
        kos_printf("------------------------------\n");

        bme280_status = bme280_get_sensor_data(BME280_ALL, &sensor_data, &bme280_sensor);
        bme280_assert(bme280_status, "Failed to get sensor data");

        temperature = sensor_data.temperature;      // ºC
        pressure    = sensor_data.pressure*0.01;    // hPa
        humidity    = sensor_data.humidity;         // %

        kos_printf("Temperature: %f ºC\n", temperature);
        kos_printf("Pressure:    %f hPa\n", pressure);
        kos_printf("Humidity:    %f %%\n", humidity);

        temperature = (9.0/5.0)*temperature + 32.0;     // ºF
        float pressure_psi = pressure/68.94757;         // psi

        kos_printf("------------------------------\n");

        kos_printf("Temperature: %f ºF\n", temperature);
        kos_printf("Pressure:    %f psi\n", pressure_psi);
        kos_printf("Humidity:    %f %%\n", humidity);

        kos_printf("------------------------------\n");

        float pressure_altitude = 145336.45*( 1 - pow(pressure/1013.25, 0.190284));  // ft
        float alt_setting = 30.23;      // in Hg
        float altitude = pressure_altitude - 1000*(29.92 - alt_setting);        // ft

        kos_printf("Pressure altitutde: %.0f ft\n", pressure_altitude);
        kos_printf("Altitude: %.0f ft\n", altitude);

        kos_printf("==============================\n");

    } while(0);

    kos_tcb_suspend(KOS_ROOT_SLOT_TCB);
    return 0;

}

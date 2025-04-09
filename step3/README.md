
# Step 3:  Sensor Implementation

In this step, we will implement the [BME280 Atmospheric Sensor](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/).  Of course, we will use an easy to use [breakout version](https://www.sparkfun.com/sparkfun-atmospheric-sensor-breakout-bme280-qwiic.html).

## Get the drivers

Download the drivers from the [Bosch Sensortec GitHub repository](https://github.com/boschsensortec/BME280_SensorAPI).  At the bottom of the README for that repository is a link for the [data sheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf).  Get that too.

We are going to put the BME280 drivers in a ``drivers`` directory.

```
Weather-Station-Tutorial
├── drivers
    └── bme280
        ├── LICENSE
        ├── README.md
        ├── bme280.c
        ├── bme280.h
        ├── bme280_defs.h
        ├── docs
        │   └── bst-bme280-ds002.pdf
        └── examples
```

## Build Configuration

Things are mostly setup already.  We really only need to include the driver in the CMake build.  This is done in 

```cmake
 # Include libraries as needed. One per include statement:
 include(kos_i2c_client)
 # include(kosup)
+include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../drivers/bme280)
 
 set(I2C "i2c")
 set(I2C_CLIENT "-L$ENV{KOS_BUILTINS_PATH}/lib -li2c_client")
 
+file(GLOB bme280_files ${CMAKE_CURRENT_SOURCE_DIR}/../drivers/bme280/*.c)
 file(GLOB deps src/*.c)
 
 add_executable(wx_sensor ${deps} ${bme280_files})
```

The first line adds the BME280 drivers for all ``#include`` directives.  The second includes all source files in the compile. 

## ``wx_sensor`` source code

In the code, we want to read the sensor data and do a calculation to see if we can determine our altitude.

Let's start at the top.

### Include and define directives.

We will add both some standard C libraries to do math and the headers for the  BME280 driver.

```c
+#include <stdlib.h>
+#include <math.h>

 #include <kos.h>
 #include <kos_utils.h>
+#include "../../drivers/bme280/bme280.h"
+#include "../../drivers/bme280/bme280_defs.h"
```
We will need two things:  a delay in microsecond and the I2C address of the BME280.  

```
+#define DELAY           1000000
+#define BME280_ADDR     BME280_I2C_ADDR_SEC
+
 enum token_slot_index {
     TOKEN_CLIENT_TRANSPORT = 1,
     TOKEN_I2C
 };
```

The I2C address is the address on the I2C bus that we will find the device.  There are two choices: ``0x76`` and ``0x77``.  For our sensors, out of the box, the standard one is ``0x77``.  You can change this to the other one by setting a jumper on the board.  The address is set by the ``BME280_I2C_ADDR_SEC`` in ``bme280_defs.h``.

### Create functions to read/write/delay from the BME280

We will need three functions to read, write, and delay the BME280.  This is discussed in the BME280 sheet on page 57.  The code there won't work for us because we are using KOS.  Instead, we will use some KOS specific functions.

#### Read

```c
-int main(int argc, char *argv[]) {
+int8_t bme280_i2c_bus_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr)
+{
+    kos_i2c_write_synchronous(TOKEN_I2C, BME280_ADDR, &reg_addr, 1);
+    kos_i2c_read_synchronous(TOKEN_I2C, BME280_ADDR, data, len);
+    return 0;
+}
```

These KOS functions seem sensible.  It won't work unless you write to the device first. 

#### Write 

```c
+int8_t bme280_i2c_bus_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr)
+{
+    uint8_t *buf;
+
+    buf = malloc(len + 1);
+    buf[0] = reg_addr;
+    memcpy(buf + 1, data, len);
+
+    if (kos_i2c_write_synchronous(TOKEN_I2C, BME280_ADDR, buf, len + 1) < (uint16_t) len) 
+    {
+        return BME280_E_COMM_FAIL;
+    }
+
+    free(buf);
+
+    return BME280_OK;
+}
```

This is more complicated, and is written to handle data of arbitrary size.  In reality, that is probably unnecessary.  We could modify this later, if we feel like it.

#### Delay

We are required to have a delay function.  This one uses ``kos_delay``.

```c
+void bme280_delay_msec(uint32_t period_msec, void *intf_ptr)
+{
+    kos_delay(period_msec);
+}
```

#### A ``BME280_OK`` assertion function

The output of the read and write functions above is a BME280 status.  We want to check this is okay.  This function makes that easier.

```c
+int bme280_assert(int status, char* message) 
+{
+    if (status != BME280_OK) {
+        kos_printf("%s\n", message);
+        return 0;
+    }
+    return 1;
+}
```

### The ``main()`` function

#### Sensor setup and initialization

There are a number of things we need to do to setup the sensor.  

- Setup the I2C interface
- Set the sensor's settings
- Make selections about what to measure when we read from the sensor

```c
     kos_printf("I2C connection established.\n");
 
+    /*
+    *   BME280 SETUP AND INITIALIZATION
+    */
+    int8_t bme280_status = BME280_OK;
+    
+    // Set the i2c interface 
+    struct bme280_dev bme280_sensor;
+    bme280_sensor.intf     = BME280_I2C_INTF;
+    bme280_sensor.read     = bme280_i2c_bus_read;
+    bme280_sensor.write    = bme280_i2c_bus_write;
+    bme280_sensor.delay_us = bme280_delay_msec;
```

The variable ``bme280_sensor`` is a structure.  The values ``read``, ``write``, and ``delay_us`` are pointers to functions.  Here we define which functions they point to, which are, of course, the functions we defined above.

There are a number of settings for how we sample humidity, pressure, and temperature.  The values given below set those for "indoor navigation".  Read the data sheet for more information on this.

```c
+    // Set sensor settings
+    struct bme280_settings settings;
+    settings.osr_h  = BME280_OVERSAMPLING_1X;
+    settings.osr_p  = BME280_OVERSAMPLING_16X;
+    settings.osr_t  = BME280_OVERSAMPLING_2X;
+    settings.filter = BME280_FILTER_COEFF_16;
```

Now we are ready to initialize the sensor.

```c
+    kos_printf("Initialize the BME280 sensor\n");
+
+    bme280_status = bme280_init(&bme280_sensor);
+    bme280_assert(bme280_status, "Failed to initialize the device");
```

Now we select which values we want to measure.

```c
+    // Set what to measure and whether to filter
+    uint8_t selection = BME280_SEL_OSR_PRESS  \
+                        | BME280_SEL_OSR_TEMP \
+                        | BME280_SEL_OSR_HUM  \
+                        | BME280_SEL_FILTER;
+
+    kos_printf("Set sensor settings\n");
```

The variable ``selection`` is a bit mask that tells the sensor which variable we want to measure.  This must be set before we make any measurements.

Here's where we do that.

```c
     /*
-    *   Talk to the I2C server
     */
-    uint32_t features;
-    status = kos_i2c_get_features(TOKEN_I2C, &features);
-    kos_assert_ok(status, "Problem with I2C features.\n");
+    bme280_status = bme280_set_sensor_settings(selection, &settings, &bme280_sensor);
+    bme280_assert(bme280_status, "Failed to get sensor settings");
 
     /* 
-    *   This should print   Features 0x00000000 
+    *   BME280 GET DATA
     */
-    kos_printf("I2C features:  0x%08x\n", features);  
```

Some of that is deleting what we had from the last step.  The new stuff sets the sensor selection and settings using ``bme280_set_sensor_settings``.  This function is in the BME280 driver.

#### Make some measurements

Now we are ready to make some measurements

```c
+    struct bme280_data sensor_data;
+    float temperature, pressure, humidity;
+    
+    do {
 
+        kos_printf("Set sensor mode\n");
+
+        // Set the mode to forced mode.  This happens  every loop.
+        bme280_status = bme280_set_sensor_mode(BME280_POWERMODE_FORCED, &bme280_sensor); 
+        bme280_assert(bme280_status, "Failed to set sensor mode");
+
+        // kos_printf("Delay for some reason");
+        // bme280_sensor.delay_us(DELAY, bme280_sensor.intf_ptr);
+
+        kos_printf("==============================\n");
+        kos_printf("Get sensor data\n");
+        kos_printf("------------------------------\n");
+
+        bme280_status = bme280_get_sensor_data(BME280_ALL, &sensor_data, &bme280_sensor);
+        bme280_assert(bme280_status, "Failed to get sensor data");
+
+        temperature = sensor_data.temperature;      // ºC
+        pressure    = sensor_data.pressure*0.01;    // hPa
+        humidity    = sensor_data.humidity;         // %
+
+        kos_printf("Temperature: %f ºC\n", temperature);
+        kos_printf("Pressure:    %f hPa\n", pressure);
+        kos_printf("Humidity:    %f %%\n", humidity);
```

That's it.  You have read from the sensor.  The units are ºC, hPa, and % relative humidity.

It would be fun to do something more with this.  Let's convert to ºF and estimate our altitude.

```c
+        // convert to ºF and inches of Hg
+        temperature = (9.0/5.0)*temperature + 32.0;    // ºF
+        float pressure_psi = pressure/68.94757;        // psi
+
+        kos_printf("------------------------------\n");
+
+        kos_printf("Temperature: %f ºF\n", temperature);
+        kos_printf("Pressure:    %f psi\n", pressure_psi);
+        kos_printf("Humidity:    %f %%\n", humidity);
```

We now have temperature in ºF and pressure in psi.

To estimate altitude we need to know the pressure altitude.  Pressure altitude is the altitude you would be at for a standard atmosphere at standard barometric pressure.  The formal for this is

$$
{\displaystyle h=145366.45\left[1-\left({\frac {\text{Station pressure in hPa}}{1013.25}}\right)^{0.190284}\right].}
$$

This give pressure altitude in feet.

Finally, the barometric pressure changes all the time; that is, it's not necessarily standard.  You can find the value by Googling "altimeter setting" for an airport near you (try KPIT, PIT is Pittsburgh International Airport, the K says it is, well, and airport).  The day I wrote this the altimeter setting at KPIT was 30.06 in Hg.  The standard altimeter setting is 29.92 in Hg.  If you use this, your altitue will be the same as the pressure altitude.  Altimeter settings change hourly, so use as recent a value as you can.

The altitude for a non-standard altimeter setting is 

$$
\text{Altitude} = \text{Pressure altitude} - 1000 \left( 29.92 - \text{altimeter setting} \right)
$$

This is for both alitudes in units of feet and the altimeter setting in inches of Hg.

```c
+        kos_printf("------------------------------\n");
+
+        float pressure_altitude = 145336.45*( 1 - pow(pressure/1013.25, 0.190284));
+        float alt_setting = 30.06;     // this changes so set it when you run the program
+        float altitude = pressure_altitude - 1000*(29.92 - alt_setting);
+
+        kos_printf("Pressure altitutde: %.0f ft\n", pressure_altitude);
+        kos_printf("Altitude: %.0f ft\n", altitude);
+
+        kos_printf("==============================\n");
+
+    } while(0);
+
     kos_tcb_suspend(KOS_ROOT_SLOT_TCB);
     return 0;
 ```

 Finally, we wrapped all of this in a ``do .. while()`` so that later we can repeat the measurement process.
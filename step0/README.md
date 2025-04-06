
# Step 0:  Initial Setup

## Hardware setup

A drawing of the pins on a BeableBone Black can be found [here](https://community.element14.com/resized-image/__size/919x544/__key/communityserver-blogs-components-weblogfiles/00-00-00-00-80/4682.contentimage_5F00_130433.png).

- **Supply power** to the BeagleBone using a USB mini connector
- Connect the **USB to Serial TTL** line.  
- Connect the Qwiic line
    - Pin 1 to GND (Qwiic black)
    - Pin 3 to 3.3V (Qwiic red)
    - Pin 19 to SCL (Qwiic yellow)
    - Pin 20 to SDA (Qwiic blue)

The serial line and Qwiic connections look like this.

<img src="images/connections.png" alt="Wiring Image" width="400"/>

## Software setup

### Create a new system with an app

Create a new system.

```terminal
$ mix kos.new.system wx_station
```

### Build and check

Build and run to make sure it works.

```terminal
$ mix kos.build
$ kos-run --iface $IFACE --serial $SERIAL
```


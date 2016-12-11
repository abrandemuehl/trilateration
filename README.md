# Attempted ESP8266 Based WiFi Trilateration

This was a project done for UIUC's ECE 445 - Senior Design.

This project does not trilaterate positions succesfully - code for communication is still needed.

The program writes the MAC address and the associated RSSI of packets received in promiscuous mode to the UART connection at 115200 BAUD.
The messages are in a binary format found in `common/messages.h`.
They are read back by the PC program and displayed like so
```
WIFI:   6c:72:e7:d1:12:4d 40
Serial: 6c:72:e7:d1:12:4d 40
WIFI:   6c:72:e7:d1:12:4d 40
Serial: 6c:72:e7:d1:12:4d 40
WIFI:   6c:72:e7:d1:12:4d 40
Serial: 6c:72:e7:d1:12:4d 40
WIFI:   6c:72:e7:d1:12:4d 40
Serial: 6c:72:e7:d1:12:4d 40
WIFI:   6c:72:e7:d1:12:4d 40
Serial: 6c:72:e7:d1:3f:e2 6
WIFI:   6c:72:e7:d1:3f:e2 6
Serial: 6c:72:e7:d1:3f:e2 6
WIFI:   6c:72:e7:d1:3f:e2 6
Serial: 40:78:e7:d1:3f:e2 6
WIFI:   40:78:e7:d1:3f:e2 6
Serial: 2c:78:e7:d1:3f:e2 6
```
Messages prefaced with `WiFi` were received by the incomplete communication code.
The `Serial` came from the ESP8266.


There are some useful code snippets that can be used in other ESP8266 based projects.
The hardware design files also might be somewhat useful for reference.

The original goal of the project was to create a way to track WiFi devices indoors.
Our strategy was to enter promiscuous mode, and look at the RSSI of received packets, and use that to estimate the distance of the transmitting device from the ESP.

Boards were designed as part of the requirements for the class, but they are almost hardware equivalent to most simple ESP modules that you can purchase online for a lower price...
Also, the boards that we created have problems that we couldn't track down. We're pretty sure they were caused by poor fabrication, but there may be some flaw in the original designs.

## Building and running

Note: These instructions are for linux only.
We ran into issues with the toolchain on Mac that had to do with the size of the binary created not fitting into iram.


First, clone with the submodules included
```bash
$ git clone --recursive https://github.com/abrandemuehl/trilateration.git
```
Or, if you've already cloned it without submodules, use
```bash
$ git submodule update --init --recursive
```

Then, edit the makefile for the submodule [esp-open-sdk](https://github.com/pfalcon/esp-open-sdk) to use the 0.9.4 version.
This can be done by editing the line that says
```
VENDOR_SDK = x.x.x
```
to be
```
VENDOR_SDK = 0.9.4
```
then run `make` in the esp/esp-open-sdk directory. (This step takes a while)

Once that is complete, you can build the main app.

First, create a directory to hold the binary files `mkdir esp/bin`.

To build the main application, you first have to `source .env` to get the paths of the toolchain commands.
Once you've done that, run `make` in the `esp` directory.

Then you can run `make flash` to use the esptool to flash it onto the ESP8266 module of your choice that is in program mode.

## Strategy
Trilateration is a pretty simple concept.
You have 3 spheres in space with known radii, and you want to find the intersection point of all of the surfaces of the spheres.
You can use this strategy to get the location of a point, given the centers and radii of 3 spheres where the point lies on their surfaces.

See the [wikipedia](https://en.wikipedia.org/wiki/Trilateration) article and [this stackoverflow question](http://stackoverflow.com/questions/16176656/trilateration-and-locating-the-point-x-y-z) for more information about the algorithm.

The exact trilateration algorithm is implemented in `common/trilatertion.c`.



## ESP Specific Code

#### Promiscuous mode and MAC addresses
In order to use communicate the positions and radii of the circles estimated from the RSSI, each device must be identifiable through some means.
We chose the MAC address of devices, as this is *supposed* to be unique per device.
However, the information given to the promiscuous_rx_cb by the SDK is very limited on SDK versions >= 0.9.5.
It only passes some basic information about the packet to the callback, which does not include anything that can be used to identify the sender.
I'm not sure why they removed this functionality, but it exists in the SDK version 0.9.4, so we chose to use it.

When using SDK 0.9.4, we can get the whole 802.11 header of the packet, making it easy to get the MAC address of the sender and destination devices.
The basic structure of 802.11 packets is shown in `esp/network_80211.h` which we modified from the version found at https://github.com/ly0/esp8266-smartlink.
We added the structure of data frames, since that was what we wanted to use.

#### STATION mode and Promiscuous mode
The big problem we ran into with promiscuous mode was that the ESP must be in STATION mode for promiscuous mode to work.
This was a problem for our original design, since we planned to use the mesh networking capabilities of the ESP to communicate the positions and radii of the circles used for trilateration.

#### `rst cause:4`
Another problem we found was that the promiscuous mode callback runs at a high priority.
So, when you get more packets than you can process, the promiscuous callback will starve some other important task in the SDK (not sure which one) and cause a watchdog timer to reset the device.
The reset will cause the device to write `rst cause:4, boot mode: (x, x)` to the UART connection.
The `cause:4` indicates that the reset was caused by a watchdog timer going off, as documented [here](http://www.esp8266.com/viewtopic.php?p=2096#p2112).

The only way to mitigate this reset problem is to do less work in the promiscuous mode callback.
This doesn't mean you have to do less work total, you just need to delegate it to a lower priority task.

In the ESP sdk, tasks can be defined with the following lines of code
```C
#define message_procTaskPrio        1
#define message_procTaskQueueLen    2
os_event_t message_procTaskQueue[message_procTaskQueueLen];
static void message_procTask(os_event_t *event);

void ICACHE_FLASH_ATTR
user_init() {
  ...
  // Defines a task that can be started with system_os_post
  system_os_task(message_procTask, message_procTaskPrio,
                 message_procTaskQueue, message_procTaskQueueLen);
  ...
}
```

This defines, the task, but doesn't run it.
To run a task, run system_os_post in your promicuous mode callback.
```
void ICACHE_FLASH_ATTR
promisc_cb(uint8_t *buf, uint16_t len) {
  ...
  system_os_post(message_procTaskPrio, 0, 0);
  ...
}
```
When `system_os_post` is called, it will queue up the message processing task to be run once the CPU frees up from all of the system tasks that run at higher priority.

How you pass data to this task is up to you.
I used a ring buffer, but I didn't make it thread safe, so there are race conditions in there.



## PC Code

The PC code was added because of the STATION mode problem with promiscuous mode.
Without access points to connect to, each board would not be able to communicate with the other ones, making trilateration impossible.
We didn't want to require that the devices connect to a network, so we planned to send messages over USB to a laptop, then have the laptop broadcast the values.
We ran out of time before we could test if this method would work.


## Trilateration Problems
The trilateration algorithm and code that we had intended to use would not with real values based on RSSI.
The algorithm solves a system of equations using the spheres, but only finds a solution if they intersect at only one point.
This is not a good algorithm because it doesn't find a solution when the estimated spheres don't exactly line up, which won't happen in almost all cases.


## In conclusion

We ran out of time and couldn't finish the project for the class.
We have no intentions of working on this code again, but if you have any questions about the choices made in the code, feel free to open an issue.



















# Base station relay firmware

This firmware connects an ESP32 module with a GNSS receiver over serial and then sends the GNSS messages to the specified IP/host address of the caster device via UDP.

## Configuration

The ESP32 will start scanning for known networks on startup, and connect to the one with the strongest signal strength. If no known network is found in range, an access point network will be created.

To access the configuration web site, connect to the same network and go to the hostname address in a web browser, by default this is `http://gnss-rtk-base-station.local/` or `http://gnss-rtk-base-station/` depending on the browser. If you are connected to the access point network, the default IP is `192.168.4.1` in case the hostname address doesn't work.

## Receive data on caster device

To bind the incoming UDP to a virtual serial port [socat](http://www.dest-unreach.org/socat/) can be used. This is necessary when running [RTKBase](https://github.com/stefal/rtkbase) with a [Raspberry Pi](https://www.raspberrypi.com/products/) or similar since it only supports serial/USB connected GNSS receivers
Adding the following lines
to `/etc/rc.local` will automatically create the link on startup to
the ESP32 relay at `gnss-rtk-base-station.local` with receive port `3333` and send port `6666`.

```bash
socat -d -d PTY,raw,echo=0,perm=0666,link=/dev/ttyVA PTY,raw,echo=0,perm=0666,link=/dev/ttyVB &
socat -d -d UDP-LISTEN:3333 /dev/ttyVA,nonblock,echo=0,b115200,raw &
socat -d -d UDP-DATAGRAM:gnss-rtk-base-station.local:6666,reuseaddr /dev/ttyVA,nonblock,echo=0,b115200,raw &
```

If you change the ESP32 host address or ports, make sure to also change the corresponding parameters in the socat commands.

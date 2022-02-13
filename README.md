
#Prerequisites:
## Install the driver
https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers

## Install CLion

## Install PlatformIO

## Reference links
* https://www.aliexpress.com/item/32996463686.html
* https://randomnerdtutorials.com/esp32-esp8266-i2c-lcd-arduino-ide/

## Find esp port
```
ls -la /dev/cu.*
# small boards (UART driver should should be installed) 
/dev/cu.SLAB_USBtoUART
# big boards (port is 14610)
/dev/cu.usbserial-14610
```

## Monitoring
```
screen /dev/cu.SLAB_USBtoUART 115200
killall screen
```

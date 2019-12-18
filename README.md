# Introduction
Serial port to TCP server, used to debug the serial port through the network.
# Compile
make
# Usage
```ser2tcp -d dev_name -p port -b baud```
```
Options:
	     -d dev_name Serial device node， default：/dev/ttyS1
	     -p port     server port，        default：10010
	     -b baud     baud rate，          defualt：B115200
```

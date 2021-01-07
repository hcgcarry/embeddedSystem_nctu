## detail step
0. connet embedded to host with uart ,then there will have /dev/ttyUSB0
change /dev/ttyUSB0 in simshell.h to your device
1. host:base64 {fileyouwantTotransfer} > encoded.txt
2. on embedded site: cat > encoded.txt
3. host :sudo su 
3. host:./readf.o
4. embedded:ctrl+d
5. embedded:base64 -d encoded.txt > a.out
6.embedded:chmod 777 a.out
7. ./a.out





## script
embedded site:./receive.sh
host : ./transferFile.sh {filename}

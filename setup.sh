#!/bin/bash

echo -e "\e[4;32mCOMPILING MODULE\e[0m"
make

echo -e "\e[4;32mGENERATING KEY\e[0m"
openssl req -config ./openssl.cnf \
        -new -x509 -newkey rsa:2048 \
        -nodes -days 36500 -outform DER \
        -keyout "MOK.priv" \
        -out "MOK.der"

echo -e "\e[4;32mSIGNING MODULE\e[0m"
sudo mokutil --import MOK.der
kmodsign sha512 MOK.priv MOK.der tmod.ko

echo -e "\e[4;32mREBOOTING\e[0m"
sudo reboot
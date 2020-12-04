# HỆ ĐIỀU HÀNH
## ĐÔ ÁN 2. TÌM HIỂU VÀ LẬP TRÌNH LINUX KERNEL MODULE

Hình thức: đồ án 3 sinh viên <br>
Deadline: 23h55 ngày 7-12-2020 <br>
Nộp bài qua  moodle môn học, đặt tên file nộp là MSSV1_MSSV2_MSSV3.zip/rar <br>

Nội dung file nộp gồm:  báo cáo những gì mình hiểu (trình bày trên MSWORD), readme của module đã code, và mã nguồn.  <br>

Nội dung đồ án:<br>
Mục tiêu hiểu về Linux kernel module và hệ thống quản lý file và device trong linux, giao tiếp giữa tiến trình ở user space và code kernel space <br>
+ Viết một module dùng để tạo ra số ngẫu nhiên. <br>
+ Module này sẽ tạo một character device để cho phép các tiến trình ở userspace có thể open và read các số ngẫu nhiên.<br>

## TEST
```
> sudo insmod tmod.ko
> cat /dev/chardev0
```

## BUILD & INSTALL MODULE
- Create Makefile
```
# ./Makefile
KDIR = /lib/modules/`uname -r`/build
all:
  make -C $(KDIR) M=`pwd`
clean:
  make -C $(KDIR) M=`pwd` clean
```
- Create Kbuild
```
# ./Kbuild
EXTRA_CFLAGS = -Wall
obj-m = tmod.o
```
- Compile module<br>
```
> make
```
- Disable secure boot for `insmod`
```
> sudo mokutil --disable-validation
> sudo reboot
```
- Create openssl.cnf
```
# ./openssl.cnf
HOME                    = .
RANDFILE                = $ENV::HOME/.rnd 
[ req ]
distinguished_name      = req_distinguished_name
x509_extensions         = v3
string_mask             = utf8only
prompt                  = no

[ req_distinguished_name ]
countryName             = CA
stateOrProvinceName     = Quebec
localityName            = Montreal
0.organizationName      = cyphermox
commonName              = Secure Boot Signing
emailAddress            = example@example.com

[ v3 ]
subjectKeyIdentifier    = hash
authorityKeyIdentifier  = keyid:always,issuer
basicConstraints        = critical,CA:FALSE
extendedKeyUsage        = codeSigning,1.3.6.1.4.1.311.10.3.6,1.3.6.1.4.1.2312.16.1.2
nsComment               = "OpenSSL Generated Certificate"
```
- Generate & import keys
```
> openssl req -config ./openssl.cnf \
        -new -x509 -newkey rsa:2048 \
        -nodes -days 36500 -outform DER \
        -keyout "MOK.priv" \
        -out "MOK.der"
> sudo mokutil --import MOK.der
```
- Sign module
```
> kmodsign sha512 MOK.priv MOK.der tmod.ko
```
- Reboot to enroll key
```
> sudo reboot
```
- Install module
```
> sudo insmod tmod.ko
```

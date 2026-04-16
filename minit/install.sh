#!/bin/sh
mkdir /etc/minit
mkdir /etc/minit/udevd
mkdir /etc/minit/dbus
mkdir /etc/minit/seatd
mkdir /etc/minit/dhcpcd
chmod +x udevd/run
chmod +x dbus/run
chmod +x seatd/run
chmod +x dhcpcd/run
chmod +x m.poweroff
chmod +x m.reboot
cp udevd/run /etc/minit/udevd/
cp dbus/run /etc/minit/dbus/
cp seatd/run /etc/minit/seatd/
cp dhcpcd/run /etc/minit/dhcpcd/
cp minit /sbin/
cp m.reboot /sbin/
cp m.poweroff /sbin/

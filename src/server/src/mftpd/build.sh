#! /bin/sh

go clean
go build || exit 1
sudo service vsftpd stop
sudo killall mftpd
sudo ./mftpd &
sleep 1

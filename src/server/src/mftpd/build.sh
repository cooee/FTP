#! /bin/sh

go clean
go build || exit 1
sudo killall mftpd
sudo ./mftpd &
sleep 1
ftp localhost

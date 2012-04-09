#! /bin/sh

if [ "$1" = clean ];then
	go clean
	exit 0
fi

go build mftpd.go

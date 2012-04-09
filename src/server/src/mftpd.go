package main

import (
	"net"
	"log"
)

func main() {
	laddr, err := net.ResolveTCPAddr("tcp", "localhost:21")
	if err != nil {
		log.Fatal(err)
	}
	_, err = net.ListenTCP("tcp", laddr)
	if err != nil {
		log.Fatal(err)
	}
}

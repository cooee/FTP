package main

import (
	"fmt"
	"log"
	"net/ftp"
)

var FtpListenAddr = "localhost:21"

func main() {
	fmt.Println("Ftp: Server start listen at: ", FtpListenAddr)
	// start ftp server
	if err := ftp.ListenAndServe(0, 0, FtpListenAddr); err != nil {
		log.Fatalf("Ftp: Serve ftp failed at: %v, err message: %v", FtpListenAddr, err)
	}
	fmt.Println("Ftp: Server stop listen at: ", FtpListenAddr)
}

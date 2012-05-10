package ftp

import (
	"fmt"
	"os/exec"
	"runtime"
	"strings"
	"unsafe"
)

/*
#include <shadow.h>
#include <crypt.h>
#include <string.h>
#include <stdlib.h>

#cgo linux LDFLAGS: -lcrypt
*/
import "C"

// debug
func PRINTLINE() {
	r1, _, r3, _ := runtime.Caller(1)
	f := runtime.FuncForPC(r1)
	fmt.Println("DEBUG: func:", f.Name(), ",line:", r3)
}

// user c function to auth user
func AuthPasswd(name string, passwd string) bool {
	nameC := C.CString(name)
	defer C.free(unsafe.Pointer(nameC))

	passwdC := C.CString(passwd)
	defer C.free(unsafe.Pointer(passwdC))

	// get password struct
	pwd := C.getspnam(nameC)
	if pwd == nil {
		return false
	}

	// crypt passwd and compare 
	crypted := C.crypt(passwdC, pwd.sp_pwdp)
	rv := C.strcmp(crypted, pwd.sp_pwdp)
	if rv != 0 {
		return false
	}

	return true
}

// exec system command
func cmd(name string, arg ...string) ([]byte, error) {
	command := exec.Command(name, arg...)
	output, err := command.Output()
	if err != nil {
		return nil, err
	}
	output = []byte(strings.Replace(string(output), "\n", "\r\n", -1))
	return output, nil
}

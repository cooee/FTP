package ftp

import (
	// imports
	"fmt"
)

var (
// errors
)

var defaultHandlFuncEntrys = map[string]func(ResponseWriter, *Request){
	"CWD":  ChangeDir,
	"USER": Login,
	"PASS": Login,
}

func ChangeDir(w ResponseWriter, r *Request) {
	fmt.Fprint(w, "no support")
}

func Login(w ResponseWriter, r *Request) {
	switch r.cmd {
	case "USER":
		fmt.Fprintf(w, "%d %s", StatusNeedPasswd, "please specify the password")
	case "PASS":
		println(r.arg)
		fmt.Fprintf(w, "%d %s", StatusLoginSucces, "Login Successful")
	}
}

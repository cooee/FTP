package ftp

import (
	"fmt"
	"io"
	"os"
)

func WriteString(w *Response, status int, msg string) {
	resp := fmt.Sprintf("%d %s", status, msg)
	io.WriteString(w, resp)
}

func ChangeDir(w *Response, r *Request) {
	err := os.Chdir(r.arg[0])
	if err != nil {
		// deal with err
	}

	w.WriteString(StatusFileActionOk, "Directory successfully changed.")
}

func Login(w *Response, r *Request) {
	switch r.cmd {
	case "USER":
		if w.conn.user != nil {
			if w.conn.user.logined {
				w.WriteString(StatusLoginFailed, "can't not change user.")
				return
			}
			w.WriteString(StatusAskPasswd, "please specify the password.")
			return
		}
		w.conn.user = NewUser(r.arg[0])
		w.WriteString(StatusAskPasswd, "please specify the password.")
	case "PASS":
		if w.conn.user.name == "anonymous" {
			// handleAnonymousLogin()
			w.conn.user.askPasswd = false
		}
		if w.conn.user.askPasswd {
			if !AuthPasswd(w.conn.user.name, r.arg[0]) {
				w.WriteString(StatusLoginFailed, "please change passwd and try again.")
				return
			}
		}
		w.WriteString(StatusLoginSucces, "Login Successful.")
		w.conn.user.logined = true
	}
}

func Syst(w *Response, r *Request) {
	w.WriteString(StatusSystemType, "UNIX Type: L8.")
}

func Quit(w *Response, r *Request) {
	w.WriteString(StatusClosingContorl, "Goodbye.")
	w.closerAfterReply = true
}

func List(w *Response, r *Request) {
	err := w.connectDataConn()
	if err != nil {
		// deal with err
	}
	w.WriteString(StatusFileOk, "Here comes the directory listing.")
	list, err := cmd("ls", "-l")
	if err != nil {
		// deal with err
	}
	_, err = w.WriteData(list)
	if err != nil {
		// deal with err
	}

	w.closeDataConn()
	w.WriteString(StatusClosingData, "Directory send OK.")
}

func Port(w *Response, r *Request) {
	w.initDataConn(r.arg[0])
	w.WriteString(StatusCommandOk, "PORT command successful. Consider using PASV.")
}

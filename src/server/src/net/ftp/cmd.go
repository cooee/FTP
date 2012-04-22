package ftp

import (
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strconv"
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

func ChangeToUpDir(w *Response, r *Request) {
	cwd, err := os.Getwd()
	if err != nil {
		//
	}
	parent, _ := filepath.Split(cwd)
	err = os.Chdir(parent)
	if err != nil {
		//
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
	w.closeAfterReply = true
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

func Abort(w *Response, r *Request) {
}

func Passive(w *Response, r *Request) {
}

func CurrentWorkDir(w *Response, r *Request) {
	cwd, err := os.Getwd()
	if err != nil {
		// deal with err
	}

	w.WriteString(StatusPathNameCreated, "\""+cwd+"\" is current work directory")
}

func Mkdir(w *Response, r *Request) {
	err := os.MkdirAll(r.arg[0], 0666)
	if err != nil {
		//
	}

	w.WriteString(StatusPathNameCreated, "\""+r.arg[0]+"\" directory created")
}

func Rmdir(w *Response, r *Request) {
	err := os.Remove(r.arg[0])
	if err != nil {
		//
	}

	w.WriteString(StatusFileActionOk, "RMD command successful")
}

func Rename(w *Response, r *Request) {
	switch r.cmd {
	case "RNFR":
		_, err := os.Stat(r.arg[0])
		if err != nil {
			//
		}
		w.conn.reqFileList = append(w.conn.reqFileList, r.arg[0])

		w.WriteString(StatusReqestFileAction, "File exist, ready for destination name.")
	case "RNTO":
		err := os.Rename(w.conn.reqFileList[0], r.arg[0])
		if err != nil {
			//
		}

		w.conn.reqFileList = nil
		w.WriteString(StatusFileActionOk, "RNTO command OK.")
	}
}

func Delete(w *Response, r *Request) {
	err := os.Remove(r.arg[0])
	if err != nil {
		//
	}

	w.WriteString(StatusFileActionOk, "DELE command successful")
}

func ServeFile(w *Response, r *Request) {
	go serveFile(w, r)
}

func serveFile(w *Response, r *Request) {
	err := w.connectDataConn()
	if err != nil {
		// deal with err
	}
	defer w.closeDataConn()
	f, err := os.Open(r.arg[0])
	if err != nil {
		//
	}
	defer f.Close()
	size, err := f.Seek(0, os.SEEK_END)
	if err != nil {
		//
	}
	_, err = f.Seek(0, os.SEEK_SET)
	if err != nil {
		//
	}
	w.WriteString(StatusFileOk, "Opening "+w.getTransferMode()+" mode connection for "+
		r.arg[0]+"("+strconv.FormatInt(size, 10)+" bytes).")
	w.serveFile(f, size)
	w.WriteString(StatusClosingData, "Transfer completed.")
}

func ReceiveFile(w *Response, r *Request) {
	go receiveFile(w, r)
}

func receiveFile(w *Response, r *Request) {
	err := w.connectDataConn()
	if err != nil {
		//
	}
	defer w.closeDataConn()

	f, err := os.OpenFile(r.arg[0], os.O_CREATE|os.O_WRONLY, 0666)
	if err != nil {
		//
	}
	defer f.Close()
	w.WriteString(StatusFileOk, "Opening "+w.getTransferMode()+" mode connection for "+r.arg[0]+".")
	w.receiveFile(f)
	w.WriteString(StatusClosingData, "Transfer completed.")
}

func Type(w *Response, r *Request) {
	switch r.arg[0] {
	case "A":
		w.setTransferMode("ASCII")
		w.WriteString(StatusCommandOk, "Switching to ASCII mode.")
	case "I", "L":
		w.setTransferMode("BINARY")
		w.WriteString(StatusCommandOk, "Switching to Binary mode.")
	}
}

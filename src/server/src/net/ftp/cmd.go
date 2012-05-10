package ftp

import (
	"fmt"
	"io"
	"net"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

func WriteString(w *Response, status int, msg string) {
	resp := fmt.Sprintf("%d %s", status, msg)
	io.WriteString(w, resp)
}

// CWD
func ChangeDir(w *Response, r *Request) {
	// change directory
	err := os.Chdir(r.arg[0])
	if err != nil {
		w.WriteString(StatusFileUnAvailable, err.Error())
		return
	}

	w.WriteString(StatusFileActionOk, "Directory successfully changed.")
}

// CDUP
func ChangeToUpDir(w *Response, r *Request) {
	// get current work directory
	cwd, err := os.Getwd()
	if err != nil {
		w.WriteString(StatusFileUnAvailable, err.Error())
		return
	}

	// get parent directory
	parent, _ := filepath.Split(cwd)
	err = os.Chdir(parent)
	if err != nil {
		w.WriteString(StatusFileUnAvailable, err.Error())
		return
	}

	w.WriteString(StatusFileActionOk, "Directory successfully changed.")
}

// USER PASS
func Login(w *Response, r *Request) {
	// find command
	switch r.cmd {
	case "USER":
		if w.conn.user != nil {
			// if user already logined, response err
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
		// anonymous login
		if w.conn.user.name == "anonymous" {
			w.conn.user.askPasswd = false
		}
		// for anonymous login
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

// SYST
func Syst(w *Response, r *Request) {
	w.WriteString(StatusSystemType, "UNIX Type: L8.")
}

// QUIT
func Quit(w *Response, r *Request) {
	w.WriteString(StatusClosingContorl, "Goodbye.")
	w.closeAfterReply = true
}

// LIST
func List(w *Response, r *Request) {
	// get data connnection
	err := w.connectDataConn()
	if err != nil {
		w.WriteString(StatusCannotOpenDConn, err.Error())
		return
	}
	// close connection when func end
	defer w.closeDataConn()
	w.WriteString(StatusFileOk, "Here comes the directory listing.")
	// get dir data
	list, err := cmd("ls", "-l")
	if err != nil {
		w.WriteString(StatusReqAbort, err.Error())
		return
	}

	// send data to client
	_, err = w.WriteData(list)
	if err != nil {
		w.WriteString(StatusClosingData, err.Error())
		return
	}

	w.WriteString(StatusClosingData, "Directory send OK.")
}

// PORT
func Port(w *Response, r *Request) {
	// init data connection ip:port:mode
	w.initDataConn(r.arg[0], "ACTIVE")
	w.WriteString(StatusCommandOk, "PORT command successful. Consider using PASV.")
}

// ABOR
func Abort(w *Response, r *Request) {
	w.WriteString(StatusTransferAborted, "Transfer Aborted. Data connection closed.")
	w.WriteString(StatusClosingData, "ABOR commad ok.")
	w.closeDataConn()
}

// PASV
func Passive(w *Response, r *Request) {
	// get local data connection listen ip:port
	ls, err := net.ListenTCP("tcp", &net.TCPAddr{net.ParseIP("127.0.0.1"), 0})
	if err != nil {
		w.WriteString(StatusCannotOpenDConn, err.Error())
		return
	}

	// response data connection ip:port to client
	h := ls.Addr().(*net.TCPAddr).Port >> 8
	l := ls.Addr().(*net.TCPAddr).Port & 255
	addr := fmt.Sprintf("%s,%d,%d", strings.Replace(ls.Addr().(*net.TCPAddr).IP.String(), ".", ",", -1), h, l)

	// init data connection
	w.initDataConn(ls, "PASSIVE")
	w.WriteString(StatusPassiveMode, "Entering Passive Mode("+addr+")")
}

// PWD
func CurrentWorkDir(w *Response, r *Request) {
	// get current work dir
	cwd, err := os.Getwd()
	if err != nil {
		w.WriteString(StatusFileUnAvailable, err.Error())
		return
	}

	w.WriteString(StatusPathNameCreated, "\""+cwd+"\" is current work directory")
}

// MKD
func Mkdir(w *Response, r *Request) {
	// make directory
	err := os.MkdirAll(r.arg[0], 0666)
	if err != nil {
		w.WriteString(StatusFileUnAvailable, err.Error())
		return
	}

	w.WriteString(StatusPathNameCreated, "\""+r.arg[0]+"\" directory created")
}

// RMD
func Rmdir(w *Response, r *Request) {
	// remove directory
	err := os.Remove(r.arg[0])
	if err != nil {
		w.WriteString(StatusFileUnAvailable, err.Error())
		return
	}

	w.WriteString(StatusFileActionOk, "RMD command successful")
}

// RNFR RNTO
func Rename(w *Response, r *Request) {
	// find command
	switch r.cmd {
	case "RNFR":
		// get file status
		_, err := os.Stat(r.arg[0])
		if err != nil {
			w.WriteString(StatusFileUnAvailable, err.Error())
			return
		}
		w.conn.reqFileList = append(w.conn.reqFileList, r.arg[0])

		w.WriteString(StatusReqestFileAction, "File exist, ready for destination name.")
	case "RNTO":
		// rename file
		err := os.Rename(w.conn.reqFileList[0], r.arg[0])
		if err != nil {
			w.WriteString(StatusFileUnAvailable, err.Error())
			return
		}

		w.conn.reqFileList = nil
		w.WriteString(StatusFileActionOk, "RNTO command OK.")
	}
}

// DELE
func Delete(w *Response, r *Request) {
	// remove file
	err := os.Remove(r.arg[0])
	if err != nil {
		w.WriteString(StatusFileUnAvailable, err.Error())
		return
	}

	w.WriteString(StatusFileActionOk, "DELE command successful")
}

func ServeFile(w *Response, r *Request) {
	// create goroutine to deal with download file
	go serveFile(w, r)
}

func serveFile(w *Response, r *Request) {
	// get data connnection
	err := w.connectDataConn()
	if err != nil {
		w.WriteString(StatusCannotOpenDConn, err.Error())
		return
	}
	// close data connection when func end
	defer w.closeDataConn()

	// open file
	f, err := os.Open(r.arg[0])
	if err != nil {
		w.WriteString(StatusFileUnAvailable, err.Error())
		return
	}
	// close file when download finish
	defer f.Close()
	// get file size
	size, err := f.Seek(w.getOffset(), os.SEEK_END)
	if err != nil {
		w.WriteString(StatusFileUnAvailable, err.Error())
		return
	}
	_, err = f.Seek(0, os.SEEK_SET)
	if err != nil {
		w.WriteString(StatusFileUnAvailable, err.Error())
		return
	}
	w.WriteString(StatusFileOk, "Opening "+w.getTransferMode()+" mode connection for "+
		r.arg[0]+"("+strconv.FormatInt(size, 10)+" bytes).")

	// make channel to deal with err
	done := make(chan int, 1)
	// for muti-threads
	f.Seek(w.getOffset(), os.SEEK_SET)
	// serve file
	w.serveFile(f, size, done)
	finished := <-done
	if finished == 0 {
		w.conn.data.writer.Flush()
		w.WriteString(StatusClosingData, "Transfer completed.")
	}
}

func ReceiveFile(w *Response, r *Request) {
	// create goroutine to deal with upload file
	go receiveFile(w, r)
}

func receiveFile(w *Response, r *Request) {
	// get data connection
	err := w.connectDataConn()
	if err != nil {
		w.WriteString(StatusCannotOpenDConn, err.Error())
		return
	}
	// close data connection when func end
	defer w.closeDataConn()

	// open local file
	f, err := os.OpenFile(r.arg[0], os.O_CREATE|os.O_WRONLY, 0666)
	if err != nil {
		w.WriteString(StatusFileUnAvailable, err.Error())
		return
	}
	// close file when upload finish
	defer f.Close()
	w.WriteString(StatusFileOk, "Opening "+w.getTransferMode()+" mode connection for "+r.arg[0]+".")
	// recevie file
	w.receiveFile(f)
	w.WriteString(StatusClosingData, "Transfer completed.")
}

// TYPE
func Type(w *Response, r *Request) {
	// find command and swith transfer mode
	switch r.arg[0] {
	case "A":
		w.setTransferMode("ASCII")
		w.WriteString(StatusCommandOk, "Switching to ASCII mode.")
	case "I", "L":
		w.setTransferMode("BINARY")
		w.WriteString(StatusCommandOk, "Switching to Binary mode.")
	}
}

// NOOP
func Noop(w *Response, r *Request) {
	w.WriteString(StatusCommandOk, "Noop command ok.")
}

// REST
func Reset(w *Response, r *Request) {
	offset, err := strconv.ParseInt(r.arg[0], 10, 64)
	if err != nil {
		w.WriteString(StatusFileUnAvailable, err.Error())
		return
	}
	w.setOffset(offset)
	w.WriteString(StatusReqestFileAction, "Restarting at"+r.arg[0]+". Send STORE or RETRIEVE to initiate transfer.")
}

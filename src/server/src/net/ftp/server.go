package ftp

import (
	"bufio"
	"bytes"
	"errors"
	"fmt"
	"io"
	"log"
	"net"
	"runtime/debug"
	"strconv"
	"strings"
	"sync"
	"syscall"
	"time"
)

var (
	// welcome message
	welcome = "welcome..."
)

var (
	ErrHandlerTimeout = errors.New("Ftp handler timeout")
)

const (
	// default time out
	DEFAULTRTIMEOUT = 300 * time.Second
	DEFAULTWTIMEOUT = 10 * time.Second
)

type Handler interface {
	// server pi handler interface
	ServeFTP(w *Response, r *Request)
}

type HandlerFunc func(w *Response, r *Request)

func (f HandlerFunc) ServeFTP(w *Response, r *Request) {
	f(w, r)
}

func Error(w *Response, error string, status int) {
	fmt.Fprintf(w, "%d %s", status, error)
}

func NotFound(w *Response, r *Request) {
	// not implement command handler
	Error(w, "command not support", StatusCommandNotImpl)
}

func NotFoundHandler() Handler {
	return HandlerFunc(NotFound)
}

// data connection
type DataConn struct {
	mode       string           // transfer mode
	remoteAddr string           // client address
	ls         *net.TCPListener // data connection listener
	conn       *net.TCPConn     // data connection
	reader     *bufio.Reader    // read from data connection
	writer     *bufio.Writer    // write to data connection
	offset     int64            // download offset
}

// init data connection
func NewDataConn(arg interface{}, mode string) *DataConn {
	switch mode {
	case "ACTIVE":
		return &DataConn{remoteAddr: arg.(string), mode: mode}
	case "PASSIVE":
		return &DataConn{ls: arg.(*net.TCPListener), mode: mode}
	}
	return nil
}

// close data connection
func (d *DataConn) Close() {
	if d.writer != nil {
		d.writer.Flush()
		d.writer = nil
	}
	if d.conn != nil {
		d.conn.Close()
		d.conn = nil
	}
}

// ftp response
type Response struct {
	closeAfterReply bool     // when err occur close control connection
	written         int64    // write count
	conn            *Conn    // control connection
	req             *Request // ftp request
}

// write data to control connection
func (r *Response) WriteData(buf []byte) (int, error) {
	bufLen := len(buf)
	if bufLen == 0 {
		return 0, nil
	}
	r.written += int64(bufLen)
	n, err := r.conn.data.writer.Write(buf)
	if err == nil {
		if n != bufLen {
			err = io.ErrShortWrite
		}
		if err == nil {
			r.conn.data.writer.Flush()
		}
	}
	return n, err
}

// serve download
func (r *Response) serveFile(content io.ReadSeeker, size int64, done chan int) {
	// get transfer mode
	switch r.getTransferMode() {
	case "BINARY":
		// serve file
		written, err := io.CopyN(r.conn.data.writer, content, size)
		if err != nil {
			done <- 1
			return
		}
		if written != size {
			done <- 1
			return
		}
		done <- 0
	case "ASCII":
		// create a new reader to read data
		reader := bufio.NewReader(content)
		for {
			line, prefix, err := reader.ReadLine()
			if err != nil {
				if err == io.EOF {
					break
				}
				break
			}
			_, err = r.WriteData(line)
			if prefix {
				continue
			}
			io.WriteString(r.conn.data.writer, "\r\n")
		}
	}
}

// serve upload
func (r *Response) receiveFile(content io.WriteSeeker) {
	// get transfer mode
	switch r.getTransferMode() {
	case "BINARY":
		// read from data connection and write to file
		io.Copy(content, r.conn.data.reader)
	case "ASCII":
		for {
			// read one line once
			line, err := r.conn.data.reader.ReadString('\n')
			if err != nil {
				if err == io.EOF {
					break
				}
				break
			}
			io.WriteString(content, strings.Replace(line, "\r", "", -1))
		}
	}
}

func (r *Response) setOffset(offset int64) {
	r.conn.data.offset = offset
}

func (r *Response) getOffset() int64 {
	return r.conn.data.offset
}

func (r *Response) setTransferMode(mode string) {
	r.conn.transMode = mode
}

func (r *Response) getTransferMode() string {
	return r.conn.transMode
}

func (r *Response) initDataConn(arg interface{}, mode string) {
	r.conn.data = NewDataConn(arg, mode)
}

func (r *Response) getDataConn() *DataConn {
	return r.conn.data
}

func (r *Response) closeDataConn() {
	r.conn.data.Close()
}

// get passive mode data connection
func resolvePassiveDataConn(dc *DataConn) (err error) {
	if dc.ls == nil {
		return errors.New("listener not spcify")
	}

	dc.conn, err = dc.ls.AcceptTCP()
	if err != nil {
		return err
	}

	return nil
}

// get active mode data connection
func resolveActiveDataConn(dc *DataConn) error {
	if len(dc.remoteAddr) == 0 {
		return errors.New("remote addr not specify")
	}

	fields := strings.Split(dc.remoteAddr, ",")
	if len(fields) != 6 {
		return errors.New("invalid addr")
	}
	h, err := strconv.Atoi(fields[4])
	if err != nil {
		return err
	}
	l, err := strconv.Atoi(fields[5])
	if err != nil {
		return err
	}
	rAddr, err := net.ResolveTCPAddr("tcp", fmt.Sprintf("%s:%d", strings.Join(fields[:4], "."), h*256+l))
	if err != nil {
		return err
	}
	lAddr, err := net.ResolveTCPAddr("tcp", "localhost:20")
	if err != nil {
		return err
	}

	dc.conn, err = net.DialTCP("tcp", lAddr, rAddr)
	if err != nil {
		if err == syscall.EADDRINUSE {
			dc.conn, err = net.DialTCP("tcp", nil, rAddr)
			if err != nil {
				return err
			}
		}
	}

	return nil
}

// connect data connection
func (r *Response) connectDataConn() error {
	dataConn := r.getDataConn()
	if dataConn == nil {
		return errors.New("init data conn failed")
	}

	switch dataConn.mode {
	case "ACTIVE":
		err := resolveActiveDataConn(dataConn)
		if err != nil {
			return err
		}
	case "PASSIVE":
		err := resolvePassiveDataConn(dataConn)
		if err != nil {
			return err
		}
	}

	dataConn.reader = bufio.NewReader(dataConn.conn)
	dataConn.writer = bufio.NewWriter(dataConn.conn)

	return nil
}

func (r *Response) WriteString(status int, messge string) {
	resp := fmt.Sprintf("%d %s", status, messge)
	io.WriteString(r, resp)
}

func (r *Response) finishRequest() {
}

// write data to control connection
func (r *Response) Write(buf []byte) (int, error) {
	bufLen := len(buf)
	if bufLen == 0 {
		return 0, nil
	}
	r.written += int64(bufLen)
	n, err := r.conn.writer.Write(buf)
	if err == nil {
		if n != bufLen {
			err = io.ErrShortWrite
		}
		if err == nil {
			io.WriteString(r.conn.writer, "\r\n")
			r.conn.writer.Flush()
		}
	}

	return n, nil
}

type Conn struct {
	remoteAddr  string
	transMode   string
	reqFileList []string
	user        *User         // logined user
	server      *Server       // server
	control     *net.TCPConn  // contorl connection
	reader      *bufio.Reader // read data from control connection
	writer      *bufio.Writer // write data to control connection
	data        *DataConn     // data connection
}

// read ftp request
func (c *Conn) readRequest() (resp *Response, err error) {
	req, err := ReadRequest(c.reader)
	if err != nil {
		return nil, err
	}
	req.remoteAddr = c.remoteAddr

	resp = new(Response)
	resp.conn = c
	resp.req = req

	return resp, nil
}

func (c *Conn) close() {
	if c.writer != nil {
		c.writer.Flush()
		c.writer = nil
	}
	if c.data != nil {
		c.data.Close()
	}
	if c.control != nil {
		c.control.Close()
		c.control = nil
	}
}

// serve one ftp client
func (c *Conn) Serve() {
	defer func() {
		err := recover()
		if err == nil {
			return
		}

		var buf bytes.Buffer
		fmt.Fprintf(&buf, "ftp: pannic serving %v: %v\n", c.remoteAddr, err)
		buf.Write(debug.Stack())
		log.Print(buf.String())

		if c.control != nil {
			c.control.Close()
		}
	}()

	// send welcome
	c.WriteResponseMessage(StatusCommandOk, welcome)

	for {
		resp, err := c.readRequest()
		if err != nil {
			if e, ok := err.(net.Error); ok {
				if e.Timeout() {
					c.WriteResponseMessage(StatusServiceShutDown, "Timeout.")
				}
			}
			msg := "bad reuqest"
			if err == errInvalidRequest {
				msg = "500 bad Request"
			} else {
				break
			}
			fmt.Fprintf(c.control, "%s", msg)
			break
		}

		handler := c.server.Handler
		if handler == nil {
			handler = DefaultServeCmd
		}

		handler.ServeFTP(resp, resp.req)
		resp.finishRequest()
		if resp.closeAfterReply {
			break
		}
	}
	c.close()
}

// write response
func (c *Conn) WriteResponseMessage(status int, args ...interface{}) (int, error) {
	format := fmt.Sprintf("%d %%s\r\n", status)
	resp := fmt.Sprintf(format, args...)
	count, err := c.writer.WriteString(resp)
	c.writer.Flush()
	return count, err
}

// ftp interface
func ListenAndServe(rtimeout, wtimeout time.Duration, addr string) error {
	lAddr, err := net.ResolveTCPAddr("tcp", addr)
	if err != nil {
		return err
	}

	if rtimeout == 0 {
		rtimeout = DEFAULTRTIMEOUT
	}
	if wtimeout == 0 {
		wtimeout = DEFAULTWTIMEOUT
	}

	server := &Server{LAddr: lAddr, ReadTimeout: rtimeout, WriteTimeout: wtimeout}
	return server.ListenAndServe()
}

type Server struct {
	Handler      Handler
	LAddr        *net.TCPAddr
	ReadTimeout  time.Duration
	WriteTimeout time.Duration
}

// start ftp server
func (srv *Server) ListenAndServe() error {
	lAddr := srv.LAddr
	if lAddr == nil {
		// use default ftp port 21
		var e error
		lAddr, e = net.ResolveTCPAddr("tcp", "localhost:21")
		if e != nil {
			return e
		}
	}
	l, e := net.ListenTCP("tcp", lAddr)
	if e != nil {
		return e
	}
	return srv.Serve(l)
}

// serve ftp
func (srv *Server) Serve(l *net.TCPListener) error {
	defer l.Close()
	for {
		tcpConn, e := l.AcceptTCP()
		if e != nil {
			// sleep one second while acceptTCP failure
			time.Sleep(time.Second)
			continue
		}
		if srv.ReadTimeout != 0 {
			tcpConn.SetReadDeadline(time.Now().Add(srv.ReadTimeout))
		}
		if srv.WriteTimeout != 0 {
			tcpConn.SetWriteDeadline(time.Now().Add(srv.WriteTimeout))
		}
		conn, e := srv.newConn(tcpConn)
		if e != nil {
			continue
		}
		go conn.Serve()
	}
	return errors.New("never reach here")
}

// get control connection
func (srv *Server) newConn(tcpConn *net.TCPConn) (conn *Conn, err error) {
	conn = new(Conn)
	conn.remoteAddr = tcpConn.RemoteAddr().String()
	conn.server = srv
	conn.control = tcpConn
	conn.reader = bufio.NewReader(tcpConn)
	conn.writer = bufio.NewWriter(tcpConn)
	return conn, nil
}

type ServeCmd struct {
	mu        sync.RWMutex
	cmdEntrys map[string]Handler // command - handler
}

// serve command
func NewServeCmd() *ServeCmd {
	return &ServeCmd{cmdEntrys: make(map[string]Handler)}
}

var DefaultServeCmd = NewServeCmd()

func (s *ServeCmd) match(cmd string) Handler {
	if handler, ok := s.cmdEntrys[cmd]; ok {
		return handler
	}
	return nil
}

func (s *ServeCmd) handler(r *Request) Handler {
	s.mu.RLock()
	defer s.mu.RUnlock()

	h := s.match(r.cmd)
	if h == nil {
		return NotFoundHandler()
	}

	return h
}

func (s *ServeCmd) ServeFTP(w *Response, r *Request) {
	s.handler(r).ServeFTP(w, r)
}

func (s *ServeCmd) Handle(cmd string, handler Handler) {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.cmdEntrys[cmd] = handler
}

func (s *ServeCmd) HandleFunc(cmd string, handler func(w *Response, r *Request)) {
	s.Handle(cmd, HandlerFunc(handler))
}

// register handler
func HandleFunc(cmd string, handler func(w *Response, r *Request)) {
	DefaultServeCmd.HandleFunc(cmd, handler)
}

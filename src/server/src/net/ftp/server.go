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
	welcome = "welcome..."
)

var (
	ErrHandlerTimeout = errors.New("Ftp handler timeout")
)

const (
	DEFAULTRTIMEOUT = 300 * time.Second
	DEFAULTWTIMEOUT = 10 * time.Second
)

type Handler interface {
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
	Error(w, "command not support", StatusLoginFailed)
}

func NotFoundHandler() Handler {
	return HandlerFunc(NotFound)
}

type DataConn struct {
	remoteAddr string
	conn       *net.TCPConn
	reader     *bufio.Reader
	writer     *bufio.Writer
}

func NewDataConn(addr string) *DataConn {
	return &DataConn{remoteAddr: addr}
}

func (d *DataConn) Close() {
	if d.conn != nil {
		d.conn.Close()
	}
}

type Response struct {
	status           int
	closerAfterReply bool
	written          int64
	conn             *Conn
	req              *Request
}

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

func (r *Response) initDataConn(addr string) {
	r.conn.data = NewDataConn(addr)
}

func (r *Response) getDataConn() *DataConn {
	return r.conn.data
}

func (r *Response) closeDataConn() {
	r.conn.data.Close()
}

func (r *Response) connectDataConn() error {
	dataConn := r.getDataConn()
	if dataConn == nil {
		return errors.New("init data conn failed")
	}
	if len(dataConn.remoteAddr) == 0 {
		return errors.New("remote addr not specify")
	}

	fields := strings.Split(dataConn.remoteAddr, ",")
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

	dataConn.conn, err = net.DialTCP("tcp", lAddr, rAddr)
	if err != nil {
		if err == syscall.EADDRINUSE {
			dataConn.conn, err = net.DialTCP("tcp", nil, rAddr)
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
	remoteAddr string
	user       *User
	server     *Server
	control    *net.TCPConn
	reader     *bufio.Reader
	writer     *bufio.Writer
	data       *DataConn
}

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
		if resp.closerAfterReply {
			break
		}
	}
	c.close()
}

func (c *Conn) WriteResponseMessage(status int, args ...interface{}) (int, error) {
	format := fmt.Sprintf("%d %%s\r\n", status)
	resp := fmt.Sprintf(format, args...)
	count, err := c.writer.WriteString(resp)
	c.writer.Flush()
	return count, err
}

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
	cmdEntrys map[string]Handler
}

func NewServeCmd() *ServeCmd {
	return &ServeCmd{cmdEntrys: make(map[string]Handler)}
}

// stay here
/*
func NewDefaultServeCmd(entrys map[string]func(*Response, *Request)) *ServeCmd {
	serveCmd := &ServeCmd{cmdEntrys: make(map[string]Handler)}
	for k, v := range entrys {
		serveCmd.HandleFunc(k, v)
	}

	return serveCmd
}
*/

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

func HandleFunc(cmd string, handler func(w *Response, r *Request)) {
	DefaultServeCmd.HandleFunc(cmd, handler)
}

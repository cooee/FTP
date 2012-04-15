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
	"sync"
	"time"
)

var (
	welcome = "welcome..."
)

var (
	ErrHandlerTimeout = errors.New("Ftp handler timeout")
)

type Handler interface {
	ServeFTP(w ResponseWriter, r *Request)
}

type HandlerFunc func(w ResponseWriter, r *Request)

func (f HandlerFunc) ServeFTP(w ResponseWriter, r *Request) {
	f(w, r)
}

func Error(w ResponseWriter, error string, status int) {
	fmt.Fprintf(w, "%d %s", status, error)
}

func NotFound(w ResponseWriter, r *Request) {
	Error(w, "command not support", StatusSyntaxErr)
}

func NotFoundHandler() Handler {
	return HandlerFunc(NotFound)
}

type ResponseWriter interface {
	Write([]byte) (int, error)
}

type response struct {
	conn             *Conn
	req              *Request
	status           int
	written          int64
	closerAfterReply bool
}

func (resp *response) finishRequest() {
	resp.conn.writer.Flush()
}

func (resp *response) Write(buf []byte) (int, error) {
	bufLen := len(buf)
	if bufLen == 0 {
		return 0, nil
	}
	resp.written += int64(bufLen)
	n, err := resp.conn.writer.Write(buf)
	if err == nil {
		if n != bufLen {
			err = io.ErrShortWrite
		}
		if err == nil {
			io.WriteString(resp.conn.writer, "\r\n")
		}
	}

	return n, nil
}

type Conn struct {
	currentWorkDir string
	remoteAddr     string
	server         *Server
	control        *net.TCPConn
	data           *net.TCPConn
	reader         *bufio.Reader
	writer         *bufio.Writer
}

func (c *Conn) readRequest() (resp *response, err error) {
	req, err := ReadRequest(c.reader)
	if err != nil {
		return nil, err
	}
	req.remoteAddr = c.remoteAddr

	resp = new(response)
	resp.conn = c
	resp.req = req

	return resp, nil
}

func (c *Conn) close() {
	if c.writer != nil {
		c.writer.Flush()
		c.writer = nil
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

func ListenAndServe(root string, addr string) error {
	lAddr, err := net.ResolveTCPAddr("tcp", addr)
	if err != nil {
		return err
	}

	if len(root) == 0 {
		root = "/"
	}

	server := &Server{LAddr: lAddr, workDir: root}
	return server.ListenAndServe()
}

type Server struct {
	workDir      string
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
	conn.currentWorkDir = srv.workDir
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

func NewDefaultServeCmd(entrys map[string]func(ResponseWriter, *Request)) *ServeCmd {
	serveCmd := &ServeCmd{cmdEntrys: make(map[string]Handler)}
	for k, v := range entrys {
		serveCmd.HandleFunc(k, v)
	}

	return serveCmd
}

var DefaultServeCmd = NewDefaultServeCmd(defaultHandlFuncEntrys)

func (s *ServeCmd) match(cmd string) Handler {
	for k, v := range s.cmdEntrys {
		if k == cmd {
			return v
		}
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

func (s *ServeCmd) ServeFTP(w ResponseWriter, r *Request) {
	s.handler(r).ServeFTP(w, r)
}

func (s *ServeCmd) Handle(cmd string, handler Handler) {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.cmdEntrys[cmd] = handler
}

func (s *ServeCmd) HandleFunc(cmd string, handler func(w ResponseWriter, r *Request)) {
	s.Handle(cmd, HandlerFunc(handler))
}

func HandleFunc(cmd string, handler func(w ResponseWriter, r *Request)) {
	DefaultServeCmd.HandleFunc(cmd, handler)
}

func DefaultRegisterHanleFunc(entry map[string]Handler)

func TimeoutHandler(h Handler, dt time.Duration, msg string) Handler {
	f := func() <-chan time.Time {
		return time.After(dt)
	}

	return &timeoutHandler{h, f, msg}
}

type timeoutHandler struct {
	handler Handler
	timeout func() <-chan time.Time
	body    string
}

func (h *timeoutHandler) ServeFTP(w ResponseWriter, r *Request) {
	done := make(chan bool)
	tw := &timeoutWriter{w: w}
	go func() {
		h.handler.ServeFTP(w, r)
		done <- true
	}()

	select {
	case <-done:
		return
	case <-h.timeout():
		tw.mu.Lock()
		defer tw.mu.Unlock()
		tw.w.Write([]byte(h.body))
	}
	tw.timeout = true
}

type timeoutWriter struct {
	w ResponseWriter

	mu      sync.Mutex
	timeout bool
}

func (tw *timeoutWriter) Write(p []byte) (int, error) {
	tw.mu.Lock()
	timeOut := tw.timeout
	tw.mu.Unlock()
	if timeOut {
		return 0, ErrHandlerTimeout
	}

	return tw.w.Write(p)
}

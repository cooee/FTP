package ftp

import (
	// "bufio"
	// "io"
	"fmt"
	"net"
	"net/textproto"
	"strconv"
	"strings"
	"testing"
	"time"
)

type EntryType int

const (
	EntryTypeFile EntryType = iota
	EntryTypeFolder
	EntryTypeLink
)

type ServerConn struct {
	conn *textproto.Conn
	host string
}

type Entry struct {
	Name string
	Type EntryType
	Size uint64
}

type response struct {
	conn net.Conn
	c    *ServerConn
}

// Helper function to execute a command and check for the expected code
func (c *ServerConn) cmd(expected int, format string, args ...interface{}) error {
	_, err := c.conn.Cmd(format, args...)
	if err != nil {
		return err
	}

	_, _, err = c.conn.ReadCodeLine(expected)
	return err
}

func Connect(addr string) (*ServerConn, error) {
	conn, err := textproto.Dial("tcp", addr)
	if err != nil {
		return nil, err
	}

	a := strings.SplitN(addr, ":", 2)
	c := &ServerConn{conn, a[0]}

	_, _, err = c.conn.ReadCodeLine(StatusCommandOk)
	if err != nil {
		c.conn.Cmd("QUIT")
		c.conn.Close()
		return nil, err
	}

	return c, nil
}

var testListenAddr = "localhost:21"

func TestStarServer(t *testing.T) {
	go ListenAndServe(0, 0, testListenAddr)
	time.Sleep(1e9)

	c, err := Connect(testListenAddr)
	if err != nil {
		t.Fatal("connect test server failed: ", err)
	}

	if c == nil {
		t.Fatal("unexpected c == nil")
	}
}

func TestCommand(t *testing.T) {
	// start server
	go ListenAndServe(0, 0, testListenAddr)
	time.Sleep(2e9)

	// connect to server
	c, err := Connect(testListenAddr)
	if err != nil {
		t.Fatal("connect test server failed: ", err)
	}

	if c == nil {
		t.Fatal("unexpected c == nil")
	}

	testLogin(t, c)
	testSyst(t, c)
	testChangeDir(t, c)
	testChangeToUpDir(t, c)
	testEpsv(t, c)
	testOpenDataConn(t, c)
	testQuit(t, c)
}

func testSyst(t *testing.T, c *ServerConn) {
	err := c.cmd(StatusSystemType, "SYST")
	if err != nil {
		t.Fatal("SYST command failed: ", err)
	}
}

func testLogin(t *testing.T, c *ServerConn) {
	err := c.cmd(StatusAskPasswd, "USER %s", "matthew")
	if err != nil {
		t.Fatal("USER command failed: ", err)
	}

	err = c.cmd(StatusLoginSucces, "PASS %s", "maxwit")
	if err != nil {
		t.Fatal("PASS command failed: ", err)
	}
}

// Changes the current directory to the specified path.
func testChangeDir(t *testing.T, c *ServerConn) {
	err := c.cmd(StatusFileActionOk, "CWD %s", "/")
	if err != nil {
		t.Fatal("CWD command failed: ", err)
	}
}

func testQuit(t *testing.T, c *ServerConn) {
	err := c.cmd(StatusClosingContorl, "QUIT")
	if err != nil {
		t.Fatal("QUIT command failed: ", err)
	}
}

func testChangeToUpDir(t *testing.T, c *ServerConn) {
	err := c.cmd(StatusFileActionOk, "CDUP")
	if err != nil {
		t.Fatal("CDUP command failed: ", err)
	}
}

// Enter passive mode
func testEpsv(t *testing.T, c *ServerConn) {
	port, err := pasv(c)
	if err != nil {
		t.Fatal("PASV command failed: ", err)
	}

	println("passive mode port: ", port)
}

func pasv(c *ServerConn) (port int, err error) {
	c.conn.Cmd("PASV")
	_, line, err := c.conn.ReadCodeLine(StatusPassiveMode)
	if err != nil {
		return
	}
	start := strings.Index(line, "(")
	end := strings.Index(line, ")")
	remoteAddr := line[start:end]
	fields := strings.Split(remoteAddr, ",")
	if len(fields) != 6 {
		return
	}
	h, err := strconv.Atoi(fields[4])
	if err != nil {
		return
	}
	l, err := strconv.Atoi(fields[5])
	if err != nil {
		return
	}
	return h<<8 + l, nil
}

// Open a new data connection using passive mode
func testOpenDataConn(t *testing.T, c *ServerConn) (net.Conn, error) {
	conn, err := openDataConn(c)
	if err != nil {
		t.Fatal("data connection init failed: ", err)
	}

	return conn, err
}

func openDataConn(c *ServerConn) (net.Conn, error) {
	port, err := pasv(c)
	if err != nil {
		return nil, err
	}

	// Build the new net address string
	addr := fmt.Sprintf("%s:%d", c.host, port)

	conn, err := net.Dial("tcp", addr)
	if err != nil {
		return nil, err
	}

	return conn, nil
}

// Renames a file on the remote FTP server.
func testRename(t *testing.T, c *ServerConn) {
	err := c.cmd(StatusFileActionOk, "RNFR %s", "/tmp/from")
	if err != nil {
		t.Fatal("RNFR command failed: ", err)
	}

	err = c.cmd(StatusFileActionOk, "RNTO %s", "/tmp/to")
	if err != nil {
		t.Fatal("RNFR command failed: ", err)
	}
}

// Deletes a file on the remote FTP server.
func testDelete(t *testing.T, c *ServerConn) {
	err := c.cmd(StatusFileActionOk, "DELE %s", "/tmp/to")
	if err != nil {
		t.Fatal("DELE command failed: ", err)
	}
}

// Creates a new directory on the remote FTP server.
func testMakeDir(t *testing.T, c *ServerConn) {
	err := c.cmd(StatusPathNameCreated, "MKD %s", "/tmp/test")
	if err != nil {
		t.Fatal("MKD command failed: ", err)
	}
}

// Removes a directory from the remote FTP server.
func testRemoveDir(t *testing.T, c *ServerConn) {
	err := c.cmd(StatusFileActionOk, "RMD %s", "/tmp/test")
	if err != nil {
		t.Fatal("RMD command failed: ", err)
	}
}

// Sends a NOOP command. Usualy used to prevent timeouts.
func testNoOp(t *testing.T, c *ServerConn) {
	err := c.cmd(StatusCommandOk, "NOOP")
	if err != nil {
		t.Fatal("NOOP command failed: ", err)
	}
}

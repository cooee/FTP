package ftp

import (
	"bufio"
	"errors"
	"io"
	"net/textproto"
	"strings"
)

var (
	errInvalidRequest = errors.New("Ftp: invalid request")
)

// implement ftp request
type Request struct {
	remoteAddr string
	// ftp command
	cmd string
	// commnad param
	arg []string
}

func ReadRequest(b *bufio.Reader) (req *Request, err error) {
	// read text protocol
	tp := textproto.NewReader(b)
	req = new(Request)

	// read ftp request
	s, err := tp.ReadLine()
	if err != nil {
		return nil, err
	}

	index := 0
	stringReader := strings.NewReader(s)
	for {
		c, err := stringReader.ReadByte()
		if err != nil {
			return nil, err
		}
		switch c {
		// deal with abort command
		case 0377, 0364, 0362:
			index++
			continue
		default:
			stringReader.UnreadByte()
			s = s[index:]
			goto BREAK
		}
	}

BREAK:
	defer func() {
		if err == io.EOF {
			err = io.ErrUnexpectedEOF
		}
	}()

	// split command and param
	params := strings.Split(s, " ")
	if len(params) == 0 {
		return nil, errInvalidRequest
	}

	// save command and param to request
	req.cmd, req.arg = params[0], params[1:]

	return req, nil
}

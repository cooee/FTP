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

type Request struct {
	remoteAddr string
	cmd        string
	arg        []string
}

func ReadRequest(b *bufio.Reader) (req *Request, err error) {
	tp := textproto.NewReader(b)
	req = new(Request)

	s, err := tp.ReadLine()
	if err != nil {
		return nil, err
	}

	defer func() {
		if err == io.EOF {
			err = io.ErrUnexpectedEOF
		}
	}()

	params := strings.Split(s, " ")
	if len(params) == 0 {
		return nil, errInvalidRequest
	}
	req.cmd, req.arg = params[0], params[1:]

	return req, nil
}

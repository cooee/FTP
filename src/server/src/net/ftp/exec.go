package ftp

import (
	"os/exec"
	"strings"
)

func cmd(name string, arg ...string) ([]byte, error) {
	command := exec.Command(name, arg...)
	output, err := command.Output()
	if err != nil {
		return nil, err
	}
	output = []byte(strings.Replace(string(output), "\n", "\r\n", -1))
	return output, nil
}

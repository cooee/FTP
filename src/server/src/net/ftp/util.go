package ftp

import (
	"fmt"
)

func DPRINT(describe string, args interface{}) {
	fmt.Printf("Debug: %s: %v\n", describe, args)
}

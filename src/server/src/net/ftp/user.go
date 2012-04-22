package ftp

type User struct {
	name      string
	guest     bool
	logined   bool
	askPasswd bool
}

func NewUser(name string) *User {
	return &User{name: name, askPasswd: true}
}

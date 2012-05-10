package ftp

type User struct {
	// user name
	name  string
	guest bool
	// is logined
	logined bool
	// if need password
	askPasswd bool
}

// register user
func NewUser(name string) *User {
	return &User{name: name, askPasswd: true}
}

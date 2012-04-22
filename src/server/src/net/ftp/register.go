package ftp

func init() {
	HandleFunc("USER", Login)
	HandleFunc("PASS", Login)
	HandleFunc("SYST", Syst)
	HandleFunc("QUIT", Quit)
	HandleFunc("LIST", List)
	HandleFunc("PORT", Port)
	HandleFunc("CWD", ChangeDir)
}

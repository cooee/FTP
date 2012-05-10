package ftp

func init() {
	// register command handler
	HandleFunc("USER", Login)
	HandleFunc("PASS", Login)
	HandleFunc("SYST", Syst)
	HandleFunc("QUIT", Quit)
	HandleFunc("LIST", List)
	HandleFunc("PORT", Port)
	HandleFunc("CWD", ChangeDir)
	HandleFunc("CDUP", ChangeToUpDir)
	HandleFunc("ABOR", Abort)
	HandleFunc("PASV", Passive)
	HandleFunc("PWD", CurrentWorkDir)
	HandleFunc("MKD", Mkdir)
	HandleFunc("RMD", Rmdir)
	HandleFunc("RNFR", Rename)
	HandleFunc("RNTO", Rename)
	HandleFunc("DELE", Delete)
	HandleFunc("TYPE", Type)
	HandleFunc("RETR", ServeFile)
	HandleFunc("STOR", ReceiveFile)
	HandleFunc("NOOP", Noop)
	HandleFunc("REST", Reset)
}

package ftp

const (
	StatusFileOk           = 150
	StatusCommandOk        = 200
	StatusSystemType       = 215
	StatusClosingContorl   = 221
	StatusClosingData      = 226
	StatusLoginSucces      = 230
	StatusFileActionOk     = 250
	StatusPathNameCreated  = 257
	StatusAskPasswd        = 331
	StatusReqestFileAction = 350
	StatusSyntaxErr        = 500
	StatusLoginFailed      = 530
)

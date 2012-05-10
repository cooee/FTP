package ftp

const (
	// ftp response status
	StatusRestart            = 110
	StatusServiceReady       = 112
	StatusDconnAlreadyOpen   = 125
	StatusFileOk             = 150
	StatusCommandOk          = 200
	StatusCommandNotImpl     = 202
	StatusSystemStatus       = 211
	StatusDirectoryStatus    = 212
	StatusFileStatus         = 213
	StatusHelpMessage        = 214
	StatusSystemType         = 215
	StatusReadyForNewUser    = 220
	StatusClosingContorl     = 221
	StatusDConnOpenNoT       = 225
	StatusClosingData        = 226
	StatusPassiveMode        = 227
	StatusLoginSucces        = 230
	StatusFileActionOk       = 250
	StatusPathNameCreated    = 257
	StatusAskPasswd          = 331
	StatusNeedAccount        = 332
	StatusReqestFileAction   = 350
	StatusServiceShutDown    = 421
	StatusCannotOpenDConn    = 425
	StatusTransferAborted    = 426
	StatusReqFileNoToken     = 450
	StatusReqAbort           = 451
	StatusInsufficienSpace   = 452
	StatusSyntaxErr          = 500
	StatusParamArgErr        = 501
	StatusCommandNImpl       = 502
	StatusBadSeqOfCommand    = 503
	StatusCmdNotImplTParam   = 504
	StatusLoginFailed        = 530
	StatusNeedAccountForStor = 532
	StatusFileUnAvailable    = 550
	StatusStorageAlloc       = 552
	StatusFileNameNotAllow   = 553
	StatusConnReset          = 10054
	StatusCannotConnRemote   = 10060
	StatusToomanyUsers       = 10068
)

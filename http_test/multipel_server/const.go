package multiple_server

import _ "embed"

const (
	webservPort = "8080"
	validHeader = "User-Agent: curl/7.77.0\r\n" +
		"Accept: */*\r\n" +
		"Connection: close\r\n\r\n"
)

var (
	//go:embed a/index.html
	aIndexHtml []byte
	//go:embed a/sample.html
	aSampleHtml []byte
	//go:embed b/index.html
	bIndexHtml []byte
	//go:embed b/sample.html
	bSampleHtml []byte
)

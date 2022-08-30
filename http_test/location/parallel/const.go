package parallel

import _ "embed"

const (
	webservPort = "8080"
	validHeader = "Host: localhost:8080\r\n" +
		"User-Agent: curl/7.77.0\r\n" +
		"Accept: */*\r\n" +
		"Connection: close\r\n\r\n"
)

var (
	//go:embed html/a/index.html
	aIndexHtml []byte
	//go:embed html/a/sample.html
	aSampleHtml []byte
	//go:embed html/a/dir/dir.html
	aDirHtml []byte
	//go:embed html/b/index.html
	bIndexHtml []byte
	//go:embed html/b/sample.html
	bSampleHtml []byte
	//go:embed html/b/dir/dir.html
	bDirHtml []byte
)

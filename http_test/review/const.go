package review

import _ "embed"

const (
	webservPort = "8080"
	validHeader = "Host: localhost:8080\r\n" +
		"User-Agent: curl/7.77.0\r\n" +
		"Accept: */*\r\n" +
		"Connection: close\r\n\r\n"
	requestLineGetRootHTTP11 = "GET / HTTP/1.1\r\n"
)

var (
	//go:embed html/index.html
	indexHtml []byte
	//go:embed html/sample.html
	sampleHtml []byte
	//go:embed html/dir/a.html
	dirAHtml []byte
	//go:embed html/dir/b.html
	dirBHtml []byte
	//go:embed html/error.html
	errorHtml []byte
	//go:embed html/あ.html
	あHtml []byte
)

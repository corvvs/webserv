package auto_index

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
)

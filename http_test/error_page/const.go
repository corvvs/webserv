package error_page

import _ "embed"

const (
	webservPort = "8080"
	validHeader = "Host: localhost:8080\r\n" +
		"User-Agent: curl/7.77.0\r\n" +
		"Accept: */*\r\n" +
		"Connection: close\r\n\r\n"
)

var (
	//go:embed html/index.html
	indexHtml []byte
	//go:embed html/bad_request.html
	badRequestHtml []byte
	//go:embed html/http_version_not_supported.html
	httpVersionNotSupportedhtml []byte
	//go:embed html/internal_server_error.html
	internalServerErrorhtml []byte
	//go:embed html/length_required.html
	lengthRequiredhtml []byte
	//go:embed html/method_not_allowed.html
	methodNotAllowedHtml []byte
	//go:embed html/not_found.html
	notFoundhtml []byte
	//go:embed html/not_implemented.html
	notImplementedhtml []byte
	//go:embed html/payload_too_large.html
	payloadTooLargehtml []byte
	//go:embed html/request_timeout.html
	requestTimeouthtml []byte
	//go:embed html/url_too_long.html
	urlTooLonghtml []byte
)

package error_page

import (
	"http_test/client"
	"net/http"
	"reflect"
	"strings"
	"testing"
)

func TestErrorPage(t *testing.T) {
	tests := []struct {
		name       string
		request    string
		clientType string
		statusCode int
		body       []byte
	}{
		{
			// invalid header
			name:       "Bad_Request",
			request:    "GET /index.html HTTP/1.1\r\n" + "\n" + validHeader,
			clientType: "default",
			statusCode: http.StatusBadRequest,
			body:       badRequestHtml,
		},
		{

			name:       "Not_Found",
			request:    "GET /not_found_path HTTP/1.1\r\n" + validHeader,
			clientType: "default",
			statusCode: http.StatusNotFound,
			body:       notFoundhtml,
		},
		{
			name:       "Payload_Too_Large",
			request:    "POST /index.html HTTP/1.1\r\n" + "Content-Length: 24\r\n" + validHeader + strings.Repeat("a", 42),
			clientType: "default",
			statusCode: http.StatusRequestEntityTooLarge,
			body:       payloadTooLargehtml,
		},
		{
			name:       "Internal_Server_Error",
			request:    "GET /cgi/cgi.py HTTP/1.1\r\n" + validHeader,
			clientType: "default",
			statusCode: http.StatusInternalServerError,
			body:       internalServerErrorhtml,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c, err := client.NewClient(tt.clientType, tt.request, webservPort)
			if err != nil {
				t.Fatal(err)
			}
			res, err := c.Run()
			if err != nil {
				t.Fatal(err)
			}
			if res.StatusCode != tt.statusCode {
				t.Errorf("unexpected status code got = %d, want %d", res.StatusCode, tt.statusCode)
			}
			if !reflect.DeepEqual(res.Body, tt.body) {
				t.Errorf("unexpected body got = %s, want %s", string(res.Body), string(tt.body))
			}
		})
	}
}

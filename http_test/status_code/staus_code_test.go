package status_code

import (
	"http_test/client"
	"net/http"
	"strings"
	"testing"
)

func longString() string {
	return strings.Repeat("0123456789", 1000)
}

func TestStatusCode(t *testing.T) {
	tests := []struct {
		name       string
		request    string
		statusCode int
	}{
		{

			name:       "Not_Implemented",
			request:    "Not_Implemented /all HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusNotImplemented,
		},
		{

			name:       "MethodNotAllowed",
			request:    "POST /only_get/ HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusMethodNotAllowed,
		},
		{
			name:       "multiple_method",
			request:    "GET GET / HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusBadRequest,
		},
		{
			name:       "different_separator",
			request:    "GET\t\tHTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusBadRequest,
		},
		{
			name:       "lower_case_http",
			request:    "GET / http/1.1\r\n" + validHeader,
			statusCode: http.StatusHTTPVersionNotSupported,
		},
		{
			name:       "http_version_0.0",
			request:    "GET / HTTP/0.0\r\n" + validHeader,
			statusCode: http.StatusHTTPVersionNotSupported,
		},
		{
			name:       "long_path",
			request:    "GET /" + longString() + " HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusRequestURITooLong,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			c, err := client.NewClient("default", tt.request, webservPort)
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
		})
	}
}

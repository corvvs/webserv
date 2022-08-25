package cgi

import (
	"http_test/client"
	"net/http"
	"reflect"
	"testing"
)

func TestCGI(t *testing.T) {
	tests := []struct {
		name       string
		request    string
		statusCode int
		body       []byte
	}{
		{

			name:       "execute_python",
			request:    "GET /execute/cgi.py HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       sampleHtml,
		},
		{

			name:       "execute_ruby",
			request:    "GET /execute/cgi.rb HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       sampleHtml,
		},
		{

			name:       "not_execute_python",
			request:    "GET /not_execute/cgi.py HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       cgiPy,
		},
		{

			name:       "not_execute_ruby",
			request:    "GET /not_execute/cgi.rb HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       cgiRb,
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
			if !reflect.DeepEqual(res.Body, tt.body) {
				t.Errorf("unexpected body got = %s, want %s", string(res.Body), string(tt.body))
			}
		})
	}
}

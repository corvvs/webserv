package experiment

import (
	"http_test/client"
	"net/http"
	"reflect"
	"testing"
)

func TestStatusOK(t *testing.T) {
	tests := []struct {
		name       string
		request    string
		statusCode int
		body       []byte
	}{
		{
			name:       "root",
			request:    "GET / HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       indexHtml,
		},
		{
			name:       "index.html",
			request:    "GET /index.html HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       indexHtml,
		},
		{
			name:       "sample.html",
			request:    "GET /sample.html HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       sampleHtml,
		},
		{
			name:       "あ.html",
			request:    "GET /あ.html HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       あHtml,
		},
		{
			name:       "dir/a.html",
			request:    "GET /dir/a.html HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       dirAHtml,
		},
		{
			name:       "dir/b.html",
			request:    "GET /dir/b.html HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       dirBHtml,
		},
		{
			// apacheははじく
			// nginxははじかない
			name:       "consecutive_whitespace",
			request:    "GET  / HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       indexHtml,
		},
		{
			// apacheははじく
			// nginxははじかない
			name:       "consecutive_head_crlf",
			request:    "\r\n\r\n" + requestLineGetRootHTTP11 + validHeader,
			statusCode: http.StatusOK,
			body:       indexHtml,
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

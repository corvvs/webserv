package experiment

import (
	"github.com/stretchr/testify/assert"
	"http_test/client"
	"net/http"
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
			c, err := client.NewClient(tt.request, webservPort)
			if err != nil {
				t.Fatal(err)
			}
			res, err := c.Run()
			if err != nil {
				t.Fatal(err)
			}
			assert.Equal(t, tt.statusCode, res.StatusCode, "unexpected status code")
			assert.Equal(t, tt.body, res.Body, "unexpected body")
		})
	}
}

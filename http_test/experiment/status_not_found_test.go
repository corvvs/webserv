package experiment

import (
	"github.com/stretchr/testify/assert"
	"http_test/client"
	"net/http"
	"testing"
)

func TestStatusStatusNotFound(t *testing.T) {
	tests := []struct {
		name       string
		request    string
		statusCode int
		body       []byte
	}{
		{
			name:       "no_such_file",
			request:    "GET /no_such_file HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusNotFound,
			body:       errorHtml,
		},
		{
			name:       "no_such_dir",
			request:    "GET /no_such_dir HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusNotFound,
			body:       errorHtml,
		},
		{
			name:       "no_such_dir/no_such_file",
			request:    "GET /no_such_dir/no_such_file HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusNotFound,
			body:       errorHtml,
		},
		{
			name:       "dir/no_such_file",
			request:    "GET /no_such_dir/no_such_file HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusNotFound,
			body:       errorHtml,
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

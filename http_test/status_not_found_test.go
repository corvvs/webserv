package main

import (
	"github.com/stretchr/testify/assert"
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
			client, err := NewClient(tt.request, webservPort)
			if err != nil {
				t.Fatal(err)
			}
			res, err := client.Run()
			if err != nil {
				t.Fatal(err)
			}
			assert.Equal(t, tt.statusCode, res.statusCode, "unexpected status code")
			assert.Equal(t, tt.body, res.body, "unexpected body")
		})
	}
}

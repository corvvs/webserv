package experiment

import (
	"http_test/client"
	"net/http"
	"reflect"
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
			if res.StatusCode != tt.statusCode {
				t.Errorf("unexpected status code got = %d, want %d", res.StatusCode, tt.statusCode)
			}
			if !reflect.DeepEqual(res.Body, tt.body) {
				t.Errorf("unexpected body got = %s, want %s", string(res.Body), string(tt.body))
			}
		})
	}
}

package status_code

import (
	"http_test/client"
	"net/http"
	"testing"
)

func TestStatusCode(t *testing.T) {
	tests := []struct {
		name       string
		request    string
		statusCode int
	}{
		{

			name:       "Not_Implemented",
			request:    "hoge /all HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusNotImplemented,
		},
		{

			name:       "Not_Implemented",
			request:    "POST /get HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusMethodNotAllowed,
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

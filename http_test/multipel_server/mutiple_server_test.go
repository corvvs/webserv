package multiple_server

import (
	"http_test/client"
	"net/http"
	"reflect"
	"testing"
)

func TestMultipleServer(t *testing.T) {
	tests := []struct {
		name       string
		request    string
		statusCode int
		body       []byte
	}{
		{

			name:       "a_index",
			request:    "GET / HTTP/1.1\r\n" + "Host: a\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       aIndexHtml,
		},
		{

			name:       "a_sample",
			request:    "GET /sample.html HTTP/1.1\r\n" + "Host: a\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       aSampleHtml,
		},
		{

			name:       "b_index",
			request:    "GET / HTTP/1.1\r\n" + "Host: b\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       bIndexHtml,
		},
		{

			name:       "b_sample",
			request:    "GET /sample.html HTTP/1.1\r\n" + "Host: b\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       bSampleHtml,
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

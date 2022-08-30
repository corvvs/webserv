package simple

import (
	"http_test/client"
	"net/http"
	"reflect"
	"testing"
)

func TestSimple(t *testing.T) {
	tests := []struct {
		name       string
		request    string
		statusCode int
		body       []byte
	}{
		{
			name:       "root",
			request:    "GET /html HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       indexHtml,
		},
		{
			name:       "indexHtml",
			request:    "GET /html/index.html HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       indexHtml,
		},
		{
			name:       "sampleHtml",
			request:    "GET /html/sample.html HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       sampleHtml,
		},
		{
			name:       "dirHtml",
			request:    "GET /dir HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       dirHtml,
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
				t.Errorf("unexpected status code got = %d, want = %d", res.StatusCode, tt.statusCode)
			}
			if !reflect.DeepEqual(res.Body, tt.body) {
				t.Errorf("unexpected body got = %s, want = %s", string(res.Body), string(tt.body))
			}
		})
	}
}

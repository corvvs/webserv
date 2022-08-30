package parallel

import (
	"http_test/client"
	"net/http"
	"reflect"
	"testing"
)

func TestParallel(t *testing.T) {
	tests := []struct {
		name       string
		request    string
		statusCode int
		body       []byte
	}{
		{
			name:       "a_root",
			request:    "GET /a HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       aIndexHtml,
		},
		{
			name:       "a_indexHtml",
			request:    "GET /a/index.html HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       aIndexHtml,
		},
		{
			name:       "a_sampleHtml",
			request:    "GET /a/sample.html HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       aSampleHtml,
		},
		{
			name:       "a_dir",
			request:    "GET /a/dir/ HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       aDirHtml,
		},
		{
			name:       "b_root",
			request:    "GET /b HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       bIndexHtml,
		},
		{
			name:       "b_indexHtml",
			request:    "GET /b/index.html HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       bIndexHtml,
		},
		{
			name:       "b_sampleHtml",
			request:    "GET /b/sample.html HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       bSampleHtml,
		},
		{
			name:       "b_dir",
			request:    "GET /b/dir/ HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			body:       bDirHtml,
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

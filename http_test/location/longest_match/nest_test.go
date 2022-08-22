package longest_match

import (
	"http_test/client"
	"net/http"
	"reflect"
	"testing"
)

func TestNest(t *testing.T) {
	tests := []struct {
		name       string
		request    string
		statusCode int
		body       []byte
	}{
		{
			name:       "longest_match",
			request:    "GET /html/index.html HTTP/1.1\r\n" + validHeader,
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
				t.Errorf("unexpected status code got = %d, want = %d", res.StatusCode, tt.statusCode)
			}
			if !reflect.DeepEqual(res.Body, tt.body) {
				t.Errorf("unexpected body got = %s, want = %s", string(res.Body), string(tt.body))
			}
		})
	}
}

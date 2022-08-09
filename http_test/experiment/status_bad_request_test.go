package experiment

import (
	"github.com/stretchr/testify/assert"
	"http_test/client"
	"net/http"
	"reflect"
	"strings"
	"testing"
)

func longString() string {
	return strings.Repeat("0123456789", 1000)
}

func TestStatusBadRequestRequestLine(t *testing.T) {
	tests := []struct {
		name       string
		request    string
		statusCode int
		body       []byte
	}{
		{
			// nginxのstatus code 404
			// apacheのstatus code 501
			name:       "invalid_method",
			request:    "method / HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusBadRequest,
			body:       errorHtml,
		},
		{
			// nginxのstatus code 404
			// apacheのstatus code 501
			name:       "lower_case_method",
			request:    "get / HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusBadRequest,
			body:       errorHtml,
		},
		{
			name:       "multiple_method",
			request:    "GET GET / HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusBadRequest,
			body:       errorHtml,
		},
		{
			name:       "different_separator",
			request:    "GET\t\tHTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusBadRequest,
			body:       errorHtml,
		},
		{
			name:       "lower_case_http",
			request:    "GET / http/1.1\r\n" + validHeader,
			statusCode: http.StatusBadRequest,
			body:       errorHtml,
		},
		{
			name:       "http_version_0.0",
			request:    "GET / HTTP/0.0\r\n" + validHeader,
			statusCode: http.StatusBadRequest,
			body:       errorHtml,
		},
		{
			name:       "long_path",
			request:    "GET /" + longString() + " HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusRequestURITooLong,
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

func TestStatusBadRequestHeader(t *testing.T) {
	tests := []struct {
		name       string
		request    string
		statusCode int
		body       []byte
	}{
		{
			name:       "non_field_name",
			request:    requestLineGetRootHTTP11 + ": close\r\n" + validHeader,
			statusCode: http.StatusBadRequest,
			body:       errorHtml,
		},
		{
			name:       "non_field",
			request:    requestLineGetRootHTTP11 + "\r\n" + validHeader,
			statusCode: http.StatusBadRequest,
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

package location

import (
	"github.com/stretchr/testify/assert"
	"http_test/client"
	"net/http"
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
		{},
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
			assert.Equal(t, tt.statusCode, res.StatusCode, "unexpected status code")
			assert.Equal(t, tt.body, res.Body, "unexpected body")
		})
	}
}

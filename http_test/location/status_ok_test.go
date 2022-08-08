package location

import (
	"github.com/stretchr/testify/assert"
	"http_test/client"
	"testing"
)

func TestStatusOK(t *testing.T) {
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

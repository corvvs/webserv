package delete

import (
	"fmt"
	"http_test/client"
	"io/fs"
	"net/http"
	"os"
	"path/filepath"
	"testing"
)

func TestUpload(t *testing.T) {
	tests := []struct {
		name       string
		request    string
		statusCode int
		fileName   string
	}{
		{

			name:       "example",
			request:    "DELETE /deleted/sample HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusOK,
			fileName:   "sample",
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			file, err := os.Create(fmt.Sprintf("./deleted/%s", tt.fileName))
			if err != nil {
				t.Fatal(err)
			}
			err = file.Close()
			if err != nil {
				t.Fatal(err)
			}
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
			err = filepath.WalkDir("./deleted", func(path string, d fs.DirEntry, err error) error {
				if d.IsDir() {
					return nil
				}
				filename := filepath.Base(path)
				if filename == tt.fileName {
					t.Errorf("file did not delete")
				}
				return nil
			})
			if err != nil {
				t.Fatal(err)
			}
		})
	}
}

func TestUploadNotFound(t *testing.T) {
	tests := []struct {
		name       string
		request    string
		statusCode int
	}{
		{

			name:       "index",
			request:    "DELETE /deleted/no_such_file HTTP/1.1\r\n" + validHeader,
			statusCode: http.StatusNotFound,
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

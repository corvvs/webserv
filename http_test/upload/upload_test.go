package upload

import (
	"fmt"
	"http_test/client"
	"io"
	"io/fs"
	"net/http"
	"os"
	"path/filepath"
	"reflect"
	"testing"
)

func TestUpload(t *testing.T) {
	tests := []struct {
		name        string
		request     string
		statusCode  int
		fileContent []byte
	}{
		{

			name:        "index",
			request:     "POST /upload HTTP/1.1\r\n" + fmt.Sprintf("Content-Length: %d\r\n", len(indexHtml)) + validHeader + string(indexHtml),
			statusCode:  http.StatusCreated,
			fileContent: indexHtml,
		},
		{

			name:        "sample",
			request:     "POST /upload HTTP/1.1\r\n" + fmt.Sprintf("Content-Length: %d\r\n", len(sampleHtml)) + validHeader + string(sampleHtml),
			statusCode:  http.StatusCreated,
			fileContent: sampleHtml,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := removeFile(uploadedDirPath)
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
			fileContent, err := findFile(uploadedDirPath)
			if err != nil {
				t.Fatal(err)
			}
			if reflect.DeepEqual(tt.fileContent[:len(tt.fileContent)-1], fileContent) {
				t.Errorf("unexpected deleted file content got = %s, want %s", string(fileContent), string(fileContent))
			}
		})
	}
}

func removeFile(dirPath string) error {
	err := filepath.WalkDir(dirPath, func(path string, d fs.DirEntry, err error) error {
		if d.IsDir() {
			return nil
		}
		return os.Remove(path)
	})
	return err
}

func findFile(dirPath string) ([]byte, error) {
	var fileContent []byte
	err := filepath.WalkDir(dirPath, func(path string, d fs.DirEntry, err error) error {
		if d.IsDir() {
			return nil
		}
		file, err := os.Open(path)
		if err != nil {
			return err
		}
		defer file.Close()
		fileContent, err = io.ReadAll(file)
		if err != nil {
			return err
		}
		return nil
	})
	if err != nil {
		return nil, err
	}
	return fileContent, err
}

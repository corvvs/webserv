package upload

import (
	"fmt"
	"http_test/client"
	"os"
	"os/exec"
	"testing"
)

func TestMain(m *testing.M) {
	cmd := exec.Command("../../webserv", "./webserv.conf")
	err := cmd.Start()
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
	for {
		c, _ := client.NewClient("default", "", webservPort)
		if c != nil {
			c.Close()
			break
		}
	}
	os.Mkdir(uploadedDirPath, 0750)

	m.Run()
	cmd.Process.Kill()
}

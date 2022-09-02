package error_page

import (
	"fmt"
	"http_test/client"
	"log"
	"os"
	"os/exec"
	"testing"
)

func TestMain(m *testing.M) {
	go func() {
		_, err := exec.Command("../../webserv", "./webserv.conf").Output()
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
	}()
	for {
		c, _ := client.NewClient("default", "", webservPort)
		if c != nil {
			c.Close()
			break
		}
	}

	m.Run()
	err := exec.Command("pkill", "webserv").Run()
	if err != nil {
		log.Fatalln(err)
	}
}
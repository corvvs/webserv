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
		c, _ := client.NewClient("", webservPort)
		if c != nil {
			c.Close()
			break
		}
	}

	err := os.Chmod("./html/permission_denied.html", 000)
	if err != nil {
		log.Fatalln(err)
	}

	m.Run()
	err = exec.Command("pkill", "webserv").Run()
	if err != nil {
		log.Fatalln(err)
	}
}

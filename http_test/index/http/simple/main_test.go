package simple

import (
	"fmt"
	"http_test/client"
	"os"
	"os/exec"
	"testing"
)

func TestMain(m *testing.M) {
	//ch := make(chan struct{})
	go func() {
		err := exec.Command("../../../../webserv", "./webserv.conf").Run()
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
	}()

	for {
		c, _ := client.NewClient("default", "", "8080")
		if c != nil {
			res, err := c.Run()
			if err != nil {
				return
			}
			fmt.Println(res.Header.Get("Host"))
			break
		}
	}

	m.Run()
	err := exec.Command("pkill", "webserv").Run()
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
	}
}

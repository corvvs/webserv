package experiment

import (
	"http_test/client"
	"net/http"
	"testing"
)

func TestMain(m *testing.M) {
	go func() {
		//o, err := exec.Command("docker-compose", "up").Output()
		//if err != nil {
		//	fmt.Fprintln(os.Stderr, err)
		//	os.Exit(1)
		//}
		//fmt.Println(string(o))
		http.Handle("/", http.FileServer(http.Dir("./html")))
		http.ListenAndServe(":8080", nil)
	}()
	for {
		c, _ := client.NewClient("", webservPort)
		if c != nil {
			c.Close()
			break
		}
		//fmt.Println("pending")
	}

	m.Run()
	//err := exec.Command("docker-compose", "stop").Run()
	//if err != nil {
	//	fmt.Fprintln(os.Stderr, err)
	//}
}

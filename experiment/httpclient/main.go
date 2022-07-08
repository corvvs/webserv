package main

import (
	"flag"
	"httpclient/client"
	"log"
)

func initFlag() (string, []string) {
	port := *flag.String("port", "8080", "default port is 8080")
	flag.Parse()
	return port, flag.Args()
}

func main() {
	port, args := initFlag()
	for _, filePath := range args {

		c, err := client.NewClient(filePath, port)
		if err != nil {
			log.Fatalln(err)
		}

		err = c.Run()
		if err != nil {
			log.Fatalln(err)
		}
	}
}

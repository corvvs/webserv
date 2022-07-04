package main

import (
	"flag"
	"httpclient/client"
	"log"
)

func main() {
	address := flag.String("address", "8080", "default address is 8080")
	flag.Parse()
	for _, filePath := range flag.Args() {
		err := Run(filePath, ":"+*address)
		if err != nil {
			log.Fatalln(err)
		}
	}
}

func Run(filePath string, address string) (err error) {
	c, err := client.NewClient(filePath, address)
	if err != nil {
		return err
	}
	defer func() {
		err = c.Close()
	}()
	err = c.Run()
	if err != nil {
		return err
	}
	return err
}

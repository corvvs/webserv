package client

import (
	"fmt"
	"net/http"
)

type Client interface {
	Run() (*Response, error)
	Close() error
}

type Response struct {
	StatusCode int
	Header     http.Header
	Body       []byte
}

var newClientMap = map[string]func(request string, port string) (Client, error){}

func NewClient(clientType string, request string, port string) (Client, error) {
	newclient, ok := newClientMap[clientType]
	if ok {
		return newclient(request, port)
	}
	return nil, fmt.Errorf("unexpected clientType type")
}

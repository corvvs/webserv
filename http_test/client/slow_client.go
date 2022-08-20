package client

import (
	"bufio"
	"io"
	"net"
	"net/http"
	"time"
)

func init() {
	newClientMap["slow"] = newSlowClient
}

type SlowClient struct {
	request []byte
	conn    net.Conn
}

func newSlowClient(request string, port string) (Client, error) {
	conn, err := net.Dial("tcp", ":"+port)
	if err != nil {
		return nil, err
	}

	return &SlowClient{
		request: []byte(request),
		conn:    conn,
	}, nil
}

func (c *SlowClient) send() error {

	for _, b := range c.request {
		_, err := c.conn.Write([]byte{b})
		if err != nil {
			return err
		}
		time.Sleep(time.Millisecond * 100)
	}
	return nil
}

func (c *SlowClient) readResponse() (*http.Response, error) {
	r := bufio.NewReader(c.conn)
	response, err := http.ReadResponse(r, nil)
	if err != nil {
		return nil, err
	}
	return response, nil
}

func (c *SlowClient) Run() (*Response, error) {
	defer c.Close()
	err := c.send()
	if err != nil {
		return nil, err
	}
	response, err := c.readResponse()
	if err != nil {
		return nil, err
	}
	statusCode := response.StatusCode
	defer response.Body.Close()
	body, err := io.ReadAll(response.Body)

	if err != nil {
		return nil, err
	}
	return &Response{
		StatusCode: statusCode,
		Header:     response.Header,
		Body:       body,
	}, nil
}

func (c *SlowClient) Close() error {
	return c.conn.Close()
}

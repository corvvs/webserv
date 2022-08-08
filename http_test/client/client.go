package client

import (
	"bufio"
	"io"
	"net"
	"net/http"
)

type Client struct {
	request []byte
	conn    net.Conn
}

type Response struct {
	StatusCode int
	Header     http.Header
	Body       []byte
}

func NewClient(request string, port string) (*Client, error) {
	conn, err := net.Dial("tcp", ":"+port)
	if err != nil {
		return nil, err
	}

	return &Client{
		request: []byte(request),
		conn:    conn,
	}, nil
}

func (c *Client) send() error {
	_, err := c.conn.Write(c.request)
	if err != nil {
		return err
	}
	return nil
}

func (c *Client) readResponse() (*http.Response, error) {
	r := bufio.NewReader(c.conn)
	response, err := http.ReadResponse(r, nil)
	if err != nil {
		return nil, err
	}
	return response, nil
}

func (c *Client) Run() (*Response, error) {
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

func (c *Client) Close() error {
	return c.conn.Close()
}

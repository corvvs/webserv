package main

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
	statusCode int
	header     http.Header
	body       []byte
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
	defer c.conn.Close()
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
		statusCode: statusCode,
		header:     response.Header,
		body:       body,
	}, nil
}

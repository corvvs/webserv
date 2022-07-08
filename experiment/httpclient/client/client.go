package client

import (
	"io"
	"net"
	"os"
)

type Client struct {
	reader io.ReadCloser
	writer io.WriteCloser
	conn   net.Conn
}

func NewClient(filePath string, port string) (*Client, error) {
	reader, err := os.Open(filePath)
	if err != nil {
		return nil, err
	}

	writer, err := os.Create(filePath + "_" + port)
	if err != nil {
		return nil, err
	}

	conn, err := net.Dial("tcp", ":"+port)
	if err != nil {
		return nil, err
	}

	return &Client{
		reader: reader,
		writer: writer,
		conn:   conn,
	}, nil
}

func (c *Client) Read() ([]byte, error) {
	buf, err := io.ReadAll(c.reader)
	if err != nil {
		return nil, err
	}
	return buf, nil
}

func (c *Client) Send(buf []byte) error {
	_, err := c.conn.Write(buf)
	if err != nil {
		return err
	}
	return nil
}

func (c *Client) Receive() ([]byte, error) {
	buf, err := io.ReadAll(c.conn)
	if err != nil {
		return nil, err
	}
	return buf, nil
}

func (c *Client) Write(buf []byte) error {
	_, err := c.writer.Write(buf)
	if err != nil {
		return err
	}
	return nil
}

func (c *Client) Close() error {
	err := c.reader.Close()
	if err != nil {
		return err
	}
	err = c.writer.Close()
	if err != nil {
		return err
	}
	err = c.conn.Close()
	if err != nil {
		return err
	}
	return nil
}

func (c *Client) Run() error {
    defer c.Close()
	readBuffer, err := c.Read()
	if err != nil {
		return err
	}
	err = c.Send(readBuffer)
	if err != nil {
		return err
	}
	receiptBuffer, err := c.Receive()
	if err != nil {
		return err
	}
	err = c.Write(receiptBuffer)
	if err != nil {
		return err
	}
	err = c.Close()
	if err != nil {
		return err
	}
	return nil
}

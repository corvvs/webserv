package main

import (
	"fmt"
	"io"
	"net"
	"os"
)

// 1. IPアドレス と port を指定して、メッセージを送信する
func sendRequest(w io.Writer, request io.Reader) {
	n := 7
	rb := make([]byte, n, n)
	for {
		read_bytes, rErr := request.Read(rb)
		if read_bytes <= 0 || (rErr != nil && rErr != io.EOF) {
			break
		}
		sent, sErr := w.Write(rb[0:read_bytes])
		if sent <= 0 || sErr != nil {
			break
		}
		fmt.Println("sent", sent, "bytes:", string(rb[0:read_bytes]))
	}
}

// 2. レスポンスを受け取って表示する
// 3. タイムアウトのテスト
// 4. レスポンスを待たずに送り続ける
// 5. 並列に送る

// localhost:80
func main() {
	conn, err := net.Dial("tcp", "localhost:8080")
	if err != nil {
		fmt.Println("listen error")
	}
	sendRequest(conn, os.Stdin)
	// sendRequest(conn, "GET / HTTP/1.0\r\n\r\n")
	status, err := io.ReadAll(conn)
	if err != nil {
		fmt.Println("listen error")
	}
	fmt.Println(string(status))
}

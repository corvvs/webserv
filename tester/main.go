package main

import (
	"fmt"
	"io"
	"net"
)

// 1. IPアドレス と port を指定して、メッセージを送信する
func sendRequest(w io.Writer, request string) {
	n := 7
	bs := []byte(request)
	for i := 0; i < len(request); {
		sent, err := w.Write(bs[i : i+n])
		if sent <= 0 || err != nil {
			break
		}
		fmt.Println("sent", sent, "bytes:", string(bs[i:i+n]))
		i += sent
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
	sendRequest(conn, "GET / HTTP/1.0\r\n\r\n")
	status, err := io.ReadAll(conn)
	if err != nil {
		fmt.Println("listen error")
	}
	fmt.Println(string(status))
}

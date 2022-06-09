package main

import (
	"fmt"
	"io"
	"net"
	"sync"
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
// s = request msg

func concurrentSend(requestMsg string, sleepTime, sendBytes int) {
	var wg sync.WaitGroup
	for i := 0; i < 5; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			conn, err := net.Dial("tcp", "localhost:8080")
			if err != nil {
				fmt.Println("listen error")
			}
			for j := 0; j < len(requestMsg); {
				start := j
				end := j + sendBytes
				if j+sendBytes > len(requestMsg) {
					end = len(requestMsg)
				}

				bs := []byte(requestMsg)[start:end]
				fmt.Fprint(conn, string(bs))
				j += end - start
				fmt.Println("send: ", sendBytes, "msg: ", string(bs), conn)
			}
			status, err := io.ReadAll(conn)
			if err != nil {
				fmt.Println("listen error")
			}
			fmt.Println(string(status))
		}()
	}
	wg.Wait()
}

// localhost:80
func main() {
	// conn, err := net.Dial("tcp", "localhost:8080")
	// if err != nil {
	// 	fmt.Println("listen error")
	// }
	// sendRequest(conn, os.Stdin)
	// // sendRequest(conn, "GET / HTTP/1.0\r\n\r\n")
	// status, err := io.ReadAll(conn)
	// if err != nil {
	// 	fmt.Println("listen error")
	// }
	// fmt.Println(string(status))
	concurrentSend("GET / HTTP/1.0\r\n\r\n", 2, 2)
}

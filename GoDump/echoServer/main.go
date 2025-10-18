package main

import (
	"errors"
	"fmt"
	"io"
	"net"
	"os"
	"os/signal"
	"sync"
	"time"
)

var conns map[net.Addr]net.Conn

func EchoServer(conn net.Conn, wg *sync.WaitGroup, mtx *sync.Mutex) {
	defer wg.Done()
	defer func() {
		mtx.Lock()
		delete(conns, conn.RemoteAddr())
		mtx.Unlock()
	}()
	defer conn.Close()

	fmt.Println("Connected to", conn.RemoteAddr())
	conn.Write([]byte("Greetings from echo server!\n"))

	err := conn.SetReadDeadline(time.Time{})
	if err != nil {
		fmt.Println("Error while setting time out")
		fmt.Println(err.Error())
		return
	}

	data := make([]byte, 1024)
	for {
		// err := conn.SetReadDeadline(time.Now().Add(500 * time.Millisecond))
		n, err := conn.Read(data)
		// https://pkg.go.dev/io#Reader
		if n > 0 {
			_, err = conn.Write(data[:n])
			if err != nil {
				fmt.Println("Error while writing to connection", conn.RemoteAddr())
				fmt.Println(err.Error())
				return
			}
		}

		if err != nil {
			if errors.Is(err, os.ErrDeadlineExceeded) {
				continue
			}
			if errors.Is(err, net.ErrClosed) {
				fmt.Println("Closed connection", conn.RemoteAddr())
			} else if err == io.EOF {
				fmt.Println("Closed connection EOF", conn.RemoteAddr())
			} else {
				fmt.Println("Error while reading from connection", conn.RemoteAddr())
				fmt.Println(err)
			}
			return
		}

	}
}

func main() {
	listener, err := net.Listen("tcp4", ":6767")
	if err != nil {
		fmt.Println("Error while creating listener")
		fmt.Println(err.Error())
		return
	}
	fmt.Println("Listener @", listener.Addr())

	var wg sync.WaitGroup
	var mtx sync.Mutex

	shutdown := make(chan struct{})
	conns = make(map[net.Addr]net.Conn)

	sigc := make(chan os.Signal, 2)
	signal.Notify(sigc, os.Interrupt)
	go func() {
		<-sigc
		fmt.Println("Listener Closed!")
		close(shutdown)
		listener.Close()
		mtx.Lock()
		for _, c := range conns {
			c.Write([]byte("sybau\n"))
			c.Close()
		}
		mtx.Unlock()
	}()

	for {
		conn, err := listener.Accept()
		if err != nil {
			select {
			case <-shutdown:
				wg.Wait()
				return

			default:
				fmt.Println("Error while accepting connection")
				fmt.Println(err.Error())
			}
			continue
		}

		mtx.Lock()
		conns[conn.RemoteAddr()] = conn
		mtx.Unlock()

		wg.Add(1)
		go EchoServer(conn, &wg, &mtx)
	}

}

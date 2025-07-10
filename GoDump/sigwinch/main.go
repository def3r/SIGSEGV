package main

import (
	"fmt"
	"os"
	"os/signal"
	"syscall"

	"golang.org/x/term"
)

func CatchSIGWINCH(done chan bool, sig chan os.Signal, x, y *int) {
	for {
		select {
		case <-done:
			fmt.Print("Killed.\n")
			return
		case <-sig:
			fmt.Println("WINCH! ")
			*x, *y, _ = term.GetSize(int(os.Stdin.Fd()))
		}
	}
}

func main() {
	var x, y int
	x, y, _ = term.GetSize(int(os.Stdin.Fd()))
	done := make(chan bool)
	// defer close(done)
	sig := make(chan os.Signal, 1)
	signal.Notify(sig, syscall.SIGWINCH)

	go CatchSIGWINCH(done, sig, &x, &y)
	for _ = range 10000 {
		fmt.Print("Size: ", x, y, "\r")
	}
	fmt.Println()
	done <- true
	// for {
	// }
}

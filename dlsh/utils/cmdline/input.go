package cmdline

import (
	"errors"
	"fmt"
	"os"
	"syscall"
)

const (
	ESC         string = "\033"
	Invert      string = ESC + "[7m"
	Reset       string = ESC + "[0m"
	Up          string = ESC + "[1A"
	Down        string = ESC + "[1B"
	Right       string = ESC + "[1C"
	Left        string = ESC + "[1D"
	Save        string = ESC + " 7"
	ClLine      string = ESC + "[0J"
	ClLineToEnd string = ESC + "[0K"
	Restore     string = ESC + " 8"
)

const (
	KEY_ESCAPE          uint8 = 0x1b
	KEY_OPEN_SQ_BRACKET uint8 = 0x5b
	KEY_UP              uint8 = 0x41
	KEY_DOWN            uint8 = 0x42
	KEY_RIGHT           uint8 = 0x43
	KEY_LEFT            uint8 = 0x44
	KEY_ENTER           uint8 = 0xd
	KEY_BACKSPACE       uint8 = 0x7f
)

type Cursor struct {
	row, col uint
}

type Input struct {
	b     [256]byte
	line  []byte
	index uint
	Str   string
}

func NewInput() *Input {
	return new(Input)
}

func (inp *Input) Reset() {
	inp.line = []byte{}
	inp.index = 0
	inp.Str = ""
}

func (c *Cursor) ReflectPos() {
	fmt.Printf("\033[%d;%dH", c.row, c.col)
}

func (c *Cursor) Block() {
	fmt.Print(Invert, Reset)
}

func (c *Cursor) ReflectPosOffsetCol(colOffset uint) {
	fmt.Printf("\033[%d;%dH", c.row, c.col+colOffset)
}

func (c *Cursor) GetPos() error {
	fmt.Print("\x1b[6n")

	var buf [32]byte
	n, err := os.Stdin.Read(buf[:])
	if err != nil {
		if errors.Is(err, syscall.EAGAIN) {
			fmt.Fprintln(os.Stderr, "[tty not ready for reading]")
			return err
		} else {
			fmt.Fprintf(os.Stderr, "[tty read error: %v]\n", err)
			return err
		}
	}

	_, err = fmt.Sscanf(string(buf[:n]), "\x1b[%d;%dR", &c.row, &c.col)
	if err != nil {
		return err
	}
	return nil
}

func (c *Cursor) Reset() {
	if err := c.GetPos(); err != nil {
		fmt.Println(err)
	}
}

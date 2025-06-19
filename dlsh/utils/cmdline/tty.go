package cmdline

import (
	"fmt"
	"os"
	"slices"

	"golang.org/x/term"
)

type ClearLineMethod int8

const (
	CursorToEnd   ClearLineMethod = 0
	CursorToStart ClearLineMethod = 1
	EntireLine    ClearLineMethod = 2
)

type Tty struct {
	Inp      *Input
	Cur      *Cursor
	oldState *term.State
	err      error
}

func NewTty() *Tty {
	tty := new(Tty)
	tty.Inp = NewInput()
	tty.Cur = new(Cursor)
	tty.oldState, tty.err = term.GetState(int(os.Stdin.Fd()))
	if tty.err != nil {
		fmt.Println(tty.err)
		os.Exit(1)
	}
	return tty
}

func (tty *Tty) Raw() {
	err := &tty.err
	tty.oldState, *err = term.MakeRaw(int(os.Stdin.Fd()))
	if *err != nil {
		fmt.Println(*err)
		return
	}
}

func (tty *Tty) Restore() {
	if tty.oldState == nil {
		fmt.Println("cannot restore to a nil state")
		return
	}
	term.Restore(int(os.Stdin.Fd()), tty.oldState)
}

func (tty *Tty) Reset() {
	tty.Inp.Reset()
	tty.Cur.Reset()
}

func (tty *Tty) Read() string {
	tty.Reset()

	input := tty.Inp
	cursor := tty.Cur
	for {
		cursor.ReflectPosOffsetCol(input.index)
		cursor.Block()

		n, err := os.Stdin.Read(input.b[:])
		if n > 2 && input.b[0] == KEY_ESCAPE && input.b[1] == KEY_OPEN_SQ_BRACKET {
			// hacks from https://github.com/atomicgo/keyboard
			// hex := fmt.Sprintf("%x", inp.b[1:n])

			switch input.b[2] {
			case KEY_UP:
				// fmt.Print(up)

			case KEY_RIGHT:
				fmt.Print(Right)
				if input.index < uint(len(input.line)) {
					input.index++
				}

			case KEY_DOWN:
				// fmt.Print(down)

			case KEY_LEFT:
				fmt.Print(Left)
				if input.index > 0 {
					input.index--
				}
			}
			continue
		}

		switch input.b[0] {
		case KEY_ENTER:
			input.Str = string(input.line)
			fmt.Print("\r\n")
			return input.Str
		case KEY_BACKSPACE:
			if len(input.line) > 0 && input.index > 0 {
				input.line = slices.Delete(input.line, int(input.index)-1, int(input.index))
				input.index--
			}
		default:
			input.line = slices.Insert(input.line, int(input.index), input.b[0])
			input.index++
		}

		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}

		cursor.ReflectPos()
		tty.ClearLine(CursorToEnd)
		fmt.Printf("%s", input.line)
	}

	fmt.Print("\r\n")
	return input.Str
}

func (tty *Tty) ClearLine(cl ClearLineMethod) {
	fmt.Printf("%s[%dK", ESC, cl)
}

package cmdline

import (
	"fmt"
	"os"
	"slices"
	"strings"

	"golang.org/x/term"
)

type ClearLineMethod int8

const (
	CursorToEnd   ClearLineMethod = 0
	CursorToStart ClearLineMethod = 1
	EntireLine    ClearLineMethod = 2
)

type CliHistory struct {
	lines []string
	index uint
	size  uint
}

func (hist *CliHistory) Append(line string) {
	hist.lines = append(hist.lines, line)
	hist.size = uint(len(hist.lines))
	hist.index = hist.size - 1
}

func (hist *CliHistory) PrevLine() string {
	if hist.index > hist.size || hist.size == 0 {
		return ""
	}
	if hist.index != 0 {
		hist.index--
	}
	return hist.lines[hist.index]
}

func (hist *CliHistory) NextLine() string {
	if hist.index > hist.size || hist.size == 0 {
		return ""
	}
	if hist.index < hist.size-1 {
		hist.index++
	}
	return hist.lines[hist.index]
}

type Tty struct {
	Prompt   string
	Inp      *Input
	Cur      *Cursor
	hist     *CliHistory
	lineNum  uint
	oldState *term.State
	err      error
}

func NewTty() *Tty {
	tty := new(Tty)
	tty.Inp = NewInput()
	tty.Cur = new(Cursor)
	tty.hist = new(CliHistory)
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
	exit := false
	for {
		cursor.ReflectPos()
		tty.ClearLine(CursorToEnd)
		fmt.Printf("%s", input.line)

		cursor.ReflectPosOffsetCol(input.index)
		cursor.Block()

		n, err := os.Stdin.Read(input.b[:])
		if n > 2 && input.b[0] == KEY_ESCAPE && input.b[1] == KEY_OPEN_SQ_BRACKET {
			// hacks from https://github.com/atomicgo/keyboard
			// hex := fmt.Sprintf("%x", inp.b[1:n])

			switch input.b[2] {
			case KEY_UP:
				if tty.hist.size == 0 {
					break
				}
				if tty.hist.size == tty.lineNum {
					tty.hist.Append(string(input.line))
				}
				input.line = []byte(tty.hist.PrevLine())
				input.index = uint(len(input.line))

			case KEY_RIGHT:
				fmt.Print(Right)
				if input.index < uint(len(input.line)) {
					input.index++
				}

			case KEY_DOWN:
				if tty.hist.size == 0 {
					break
				}
				if tty.hist.size == tty.lineNum {
					tty.hist.Append(string(input.line))
				}
				input.line = []byte(tty.hist.NextLine())
				input.index = uint(len(input.line))

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
			exit = true
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

		if exit {
			break
		}

	}

	fmt.Print("\r\n")
	if tty.hist.size != tty.lineNum {
		tty.hist.lines[tty.lineNum] = input.Str
	} else {
		tty.hist.Append(input.Str)
	}
	tty.lineNum++
	return input.Str
}

func (tty *Tty) ClearLine(cl ClearLineMethod) {
	fmt.Printf("%s[%dK", ESC, cl)
}

func (tty *Tty) GetPrompt() {
	home := os.Getenv("HOME")
	cwd, err := os.Getwd()
	if err != nil {
		fmt.Println("Failed to get current dir")
		os.Exit(1)
	}
	if strings.Index(cwd, home) == 0 {
		cwd = strings.Replace(cwd, home, "~", 1)
	}
	tty.Prompt = cwd + "> "
}

func (tty *Tty) ReflectPrompt() {
	fmt.Print(tty.Prompt)
}

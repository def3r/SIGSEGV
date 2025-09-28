package main

import (
	"fmt"

	"layoutEngine/layout"
)

// Resources:
// https://arthursonzogni.github.io/FTXUI/ftxui.html
// https://third-bit.com/sdxjs/layout-engine/

func main() {
	a := layout.NewBlock(5, 5)
	b := layout.NewBlock(4, 4)
	c := layout.NewBlock(2, 2)
	col := layout.NewCol([]layout.Box{b, c})
	d := layout.NewBlock(1, 1)

	children := []layout.Box{a, col, d}
	row := layout.NewRow(children)
	row.SetMaxWidth(9)
	row.Wrap()
	row.Place(1, 1)

	s := layout.NewScreen(20, 10, true)
	s.Render(row)
	screen := s.Flush()
	screen = PostProcess(screen)
	fmt.Print(screen)
}

func PostProcess(s string) string {
	fill := "█"
	m := make(map[rune]string)
	m['a'] = "\033[0;37m"
	m['b'] = "\033[0;36m"
	m['c'] = "\033[0;34m"
	m['d'] = "\033[0;32m"
	reset := "\033[0m"
	screen := ""

	for _, r := range s {
		if _, ok := m[r]; ok {
			screen += m[r] + fill + reset
		} else {
			screen += string(r)
		}
	}
	return screen
}

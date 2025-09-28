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
	row.Place(1, 1)

	s := layout.NewScreen(20, 10, false)
	s.Render(row)
	fmt.Print(s.Flush())
}

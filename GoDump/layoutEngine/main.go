package main

import (
	"fmt"

	"layoutEngine/layout"
)

// Resources:
// https://arthursonzogni.github.io/FTXUI/ftxui.html
// https://third-bit.com/sdxjs/layout-engine/

func main() {
	b1 := layout.NewBlock(10, 5)
	fmt.Printf("Block1(%d, %d)\n", b1.GetWidth(), b1.GetHeight())
	b2 := layout.NewBlock(5, 5)
	fmt.Printf("Block2(%d, %d)\n", b2.GetWidth(), b2.GetHeight())

	children := []layout.Box{b1, b2}

	row := layout.NewRow(children)
	fmt.Printf("Row(%d, %d)\n", row.GetWidth(), row.GetHeight())

	col := layout.NewCol(children)
	fmt.Printf("Col(%d, %d)\n", col.GetWidth(), col.GetHeight())
}

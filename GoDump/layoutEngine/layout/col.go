package layout

type Col struct {
	children []Box
	x0, y0   int
}

func NewCol(children []Box) *Col {
	c := new(Col)
	c.children = append(c.children, children...)
	return c
}

func (c *Col) GetWidth() int {
	maxw := 0
	for _, child := range c.children {
		maxw = max(maxw, child.GetWidth())
	}
	return maxw
}

func (c *Col) GetHeight() int {
	sum := 0
	for _, child := range c.children {
		sum += child.GetHeight()
	}
	return sum
}

func (c *Col) Place(x0, y0 int) {
	c.x0 = x0
	c.y0 = y0
	hOffset := 0
	for _, child := range c.children {
		child.Place(x0, y0+hOffset)
		hOffset += child.GetHeight()
	}
}

func (c *Col) Render(s *Screen) {
	for _, child := range c.children {
		child.Render(s)
	}
}

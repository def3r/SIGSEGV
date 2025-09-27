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

func (r *Col) GetWidth() int {
	maxw := 0
	for _, child := range r.children {
		maxw = max(maxw, child.GetWidth())
	}
	return maxw
}

func (r *Col) GetHeight() int {
	sum := 0
	for _, child := range r.children {
		sum += child.GetHeight()
	}
	return sum
}

func (r *Col) Place(x0, y0 int) {
	r.x0 = x0
	r.y0 = y0
	hOffset := 0
	for _, child := range r.children {
		child.Place(x0, y0+hOffset)
		hOffset += child.GetHeight()
	}
}

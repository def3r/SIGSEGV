package layout

// Row (basically a hbox)
// NOTE: This should be a slice of *Box
type Row struct {
	children []Box
	x0, y0   int
}

func NewRow(children []Box) *Row {
	r := new(Row)
	r.children = append(r.children, children...)
	return r
}

func (r *Row) GetWidth() int {
	sum := 0
	for _, child := range r.children {
		sum += child.GetWidth()
	}
	return sum
}

func (r *Row) GetHeight() int {
	maxh := 0
	for _, child := range r.children {
		maxh = max(maxh, child.GetHeight())
	}
	return maxh
}

func (r *Row) Place(x0, y0 int) {
	r.x0 = x0
	r.y0 = y0
	y1 := y0 + r.GetHeight()
	wOffset := 0
	for _, child := range r.children {
		child.Place(x0+wOffset, y1-child.GetHeight())
		wOffset += child.GetWidth()
	}
}

func (r *Row) Render(s *Screen) {
	for _, child := range r.children {
		child.Render(s)
	}
}

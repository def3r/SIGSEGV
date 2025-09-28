package layout

// Row (basically a hbox)
// NOTE: This should be a slice of *Box
type Row struct {
	children []Box
	x0, y0   int
	maxWidth int // -1 takes width of contents
}

func NewRow(children []Box) *Row {
	r := new(Row)
	r.children = append(r.children, children...)
	r.maxWidth = -1
	return r
}

func (r *Row) SetMaxWidth(width int) {
	r.maxWidth = width
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

// NOTE: Assumes each child width < row max width
func (r *Row) Wrap() {
	if r.maxWidth == -1 || r.GetWidth() <= r.maxWidth {
		return
	}

	var rows [][]Box
	var currentRow []Box
	xOffset := 0
	for _, child := range r.children {
		childWidth := child.GetWidth()
		if xOffset+childWidth <= r.maxWidth {
			currentRow = append(currentRow, child)
			xOffset += childWidth
		} else {
			rows = append(rows, currentRow)
			currentRow = []Box{child}
			xOffset = childWidth
		}
	}
	rows = append(rows, currentRow)

	var newRows []Box
	for _, row := range rows {
		newRows = append(newRows, NewRow(row))
	}
	r.children = []Box{NewCol(newRows)}
}

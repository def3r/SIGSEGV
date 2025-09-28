package layout

import "testing"

func TestRowH_WBlocks(t *testing.T) {
	b1 := NewBlock(10, 5)
	b2 := NewBlock(2, 2)
	b3 := NewBlock(8, 5)
	children := []Box{b1, b2, b3}
	r := NewRow(children)

	assert(t, r.GetWidth(), 20, "wrong wt")
	assert(t, r.GetHeight(), 5, "wrong ht")
}

func TestRowPlaceBlocks(t *testing.T) {
	b1 := NewBlock(10, 5)
	b2 := NewBlock(2, 2)
	b3 := NewBlock(8, 5)
	children := []Box{b1, b2, b3}
	r := NewRow(children)
	r.Place(0, 0)

	assert(t, b1.x0, 0, "wrong x0")
	assert(t, b1.y0, 0, "wrong y0")

	assert(t, b2.x0, 10, "wrong x0")
	assert(t, b2.y0, 3, "wrong y0")

	assert(t, b3.x0, 12, "wrong x0")
	assert(t, b3.y0, 0, "wrong y0")
}

func TestRowPlaceRowOfBlocks(t *testing.T) {
	b1 := NewBlock(10, 5)
	b2 := NewBlock(2, 2)
	b3 := NewBlock(8, 5)
	children1 := []Box{b1, b2, b3}
	r1 := NewRow(children1)

	b4 := NewBlock(5, 10)
	b5 := NewBlock(5, 5)
	b6 := NewBlock(10, 2)
	children2 := []Box{b4, b5, b6}
	r2 := NewRow(children2)

	children := []Box{r1, r2}
	r := NewRow(children)
	r.Place(0, 0)

	assert(t, r.GetHeight(), 10, "wrong height")
	assert(t, r.GetWidth(), 40, "wrong width")

	assert(t, b1.x0, 0, "wrong x0")
	assert(t, b1.y0, 5, "wrong y0")

	assert(t, b2.x0, 10, "wrong x0")
	assert(t, b2.y0, 8, "wrong y0")

	assert(t, b3.x0, 12, "wrong x0")
	assert(t, b3.y0, 5, "wrong y0")

	assert(t, b4.x0, 20, "wrong x0")
	assert(t, b4.y0, 0, "wrong y0")

	assert(t, b5.x0, 25, "wrong x0")
	assert(t, b5.y0, 5, "wrong y0")

	assert(t, b6.x0, 30, "wrong x0")
	assert(t, b6.y0, 8, "wrong y0")
}

func TestRowWrap(t *testing.T) {
	b1 := NewBlock(10, 5)
	b2 := NewBlock(2, 2)
	b3 := NewBlock(8, 5)
	children1 := []Box{b1, b2, b3}
	// r1 := NewRow(children1)

	b4 := NewBlock(5, 10)
	b5 := NewBlock(5, 5)
	b6 := NewBlock(10, 2)
	children2 := []Box{b4, b5, b6}
	// r2 := NewRow(children2)

	children := append(children1, children2...)
	r := NewRow(children)
	// BUG: If parent max width is set, child Rows(r1, r2) can still grow larger!
	r.SetMaxWidth(12)
	r.Wrap()
	r.Place(0, 0)

	assert(t, r.GetHeight(), 22, "wrong height")
	assert(t, r.GetWidth(), 12, "wrong width")

	assert(t, b1.x0, 0, "wrong x0")
	assert(t, b1.y0, 0, "wrong y0")

	assert(t, b2.x0, 10, "wrong x0")
	assert(t, b2.y0, 3, "wrong y0")

	assert(t, b3.x0, 0, "wrong x0")
	assert(t, b3.y0, 5, "wrong y0")

	assert(t, b4.x0, 0, "wrong x0")
	assert(t, b4.y0, 10, "wrong y0")

	assert(t, b5.x0, 5, "wrong x0")
	assert(t, b5.y0, 15, "wrong y0")

	assert(t, b6.x0, 0, "wrong x0")
	assert(t, b6.y0, 20, "wrong y0")
}

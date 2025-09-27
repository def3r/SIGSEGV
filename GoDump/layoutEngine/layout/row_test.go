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

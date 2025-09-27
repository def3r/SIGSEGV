package layout

import "testing"

func TestColH_WBlocks(t *testing.T) {
	b1 := NewBlock(10, 5)
	b2 := NewBlock(2, 2)
	b3 := NewBlock(8, 5)
	children := []Box{b1, b2, b3}
	c := NewCol(children)

	assert(t, c.GetWidth(), 10, "wrong wt")
	assert(t, c.GetHeight(), 12, "wrong ht")
}

func TestColPlaceBlocks(t *testing.T) {
	b1 := NewBlock(10, 5)
	b2 := NewBlock(2, 2)
	b3 := NewBlock(8, 5)
	children := []Box{b1, b2, b3}
	c := NewCol(children)
	c.Place(0, 0)

	assert(t, b1.x0, 0, "wrong x0")
	assert(t, b1.y0, 0, "wrong y0")

	assert(t, b2.x0, 0, "wrong x0")
	assert(t, b2.y0, 5, "wrong y0")

	assert(t, b3.x0, 0, "wrong x0")
	assert(t, b3.y0, 7, "wrong y0")

}

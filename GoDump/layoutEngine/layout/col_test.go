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

func TestColPlaceColOfBlocks(t *testing.T) {
	b1 := NewBlock(10, 5)
	b2 := NewBlock(2, 2)
	b3 := NewBlock(8, 5)
	children1 := []Box{b1, b2, b3}
	c1 := NewCol(children1)

	b4 := NewBlock(5, 10)
	b5 := NewBlock(5, 5)
	b6 := NewBlock(10, 2)
	children2 := []Box{b4, b5, b6}
	c2 := NewCol(children2)

	children := []Box{c1, c2}
	c := NewCol(children)
	c.Place(0, 0)

	assert(t, c.GetHeight(), 29, "wrong height")
	assert(t, c.GetWidth(), 10, "wrong width")

	assert(t, b1.x0, 0, "wrong x0")
	assert(t, b1.y0, 0, "wrong y0")

	assert(t, b2.x0, 0, "wrong x0")
	assert(t, b2.y0, 5, "wrong y0")

	assert(t, b3.x0, 0, "wrong x0")
	assert(t, b3.y0, 7, "wrong y0")

	assert(t, b4.x0, 0, "wrong x0")
	assert(t, b4.y0, 12, "wrong y0")

	assert(t, b5.x0, 0, "wrong x0")
	assert(t, b5.y0, 22, "wrong y0")

	assert(t, b6.x0, 0, "wrong x0")
	assert(t, b6.y0, 27, "wrong y0")
}

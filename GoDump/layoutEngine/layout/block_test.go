package layout

import "testing"

func assert[T comparable](t *testing.T, got, expected T, msg string) {
	t.Helper()
	if got != expected {
		t.Error("Got", got, "Expected", expected, ": "+msg)
	}
}

func TestBlockH_W(t *testing.T) {
	b := NewBlock(10, 5)

	h_ := b.GetHeight()
	h_e := 5
	assert(t, h_, h_e, "mismatched hts")

	w_ := b.GetWidth()
	w_e := 10
	assert(t, w_, w_e, "mismatched width")
}

func TestBlockPlacing(t *testing.T) {
	b := NewBlock(10, 5)
	b.Place(10, 20)

	assert(t, b.x0, 10, "wrong x0")
	assert(t, b.y0, 20, "wrong y0")
}

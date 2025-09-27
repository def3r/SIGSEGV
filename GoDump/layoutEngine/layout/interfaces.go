package layout

type Box interface {
	GetWidth() int
	GetHeight() int
	Place(x, y int)
}

package layout

type Box interface {
	GetWidth() int
	GetHeight() int
	Place(int, int)
	Render(*Screen)
	Wrap()
}

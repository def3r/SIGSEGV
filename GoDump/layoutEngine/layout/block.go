package layout

// Layout Engine Implementation for dlsh
// Planned: Immediate mode rendering (with a dom?)

// Anything to put on screen is a cell
// A cell is either a Block, Row or a Col

// Block has fixed h and w
type Block struct {
	width, height int
	x0, y0        int
}

func NewBlock(width, height int) *Block {
	return &Block{width: width, height: height}
}

func (b *Block) GetWidth() int {
	return b.width
}

func (b *Block) GetHeight() int {
	return b.height
}

func (b *Block) Place(x0, y0 int) {
	b.x0 = x0
	b.y0 = y0
}

func (b *Block) Render(s *Screen) {
	fill := s.getFill()
	for h := range b.height {
		for w := range b.width {
			s.screen[b.y0+h][b.x0+w] = fill
		}
	}
}

func (b *Block) Wrap() {
	return
}

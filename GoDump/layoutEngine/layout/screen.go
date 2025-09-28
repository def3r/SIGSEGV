package layout

type Screen struct {
	width, height int
	screen        [][]rune
	fill          rune
}

func NewScreen(width, height int, fill bool) *Screen {
	s := &Screen{width: width, height: height}
	s.screen = make([][]rune, height)
	var fRune rune
	if fill {
		fRune = '.'
	} else {
		fRune = ' '
	}
	for i := range height {
		s.screen[i] = make([]rune, width)
		for j := range width {
			s.screen[i][j] = fRune
		}
	}
	return s
}

func (s *Screen) Flush() string {
	screen := ""
	for i := range s.height {
		screen = screen + string(s.screen[i])
		screen += "\n"
	}
	return screen
}

func (s *Screen) getFill() rune {
	if s.fill == 0 {
		s.fill = 'a'
	} else {
		s.fill++
	}
	return s.fill
}

func (s *Screen) Render(component Box) {
	component.Render(s)
}

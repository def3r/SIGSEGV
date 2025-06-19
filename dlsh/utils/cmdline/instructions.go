package cmdline

import (
	"fmt"
	"os"
	"os/exec"
	"slices"
	"strings"
)

type InstructionType uint8

const (
	EXEC InstructionType = iota
	PIPE
	WAIT
)

type Instruction struct {
	InsType InstructionType
	Cmd     *exec.Cmd
	R, W, E *os.File
	State   bool
}

func NewInstruction(path string, args ...string) *Instruction {
	execCmd := exec.Command(path, args...)
	execCmd.Stdout = os.Stdout
	execCmd.Stdin = os.Stdin
	execCmd.Stderr = os.Stderr

	instruction := new(Instruction)
	instruction.InsType = EXEC
	instruction.Cmd = execCmd
	instruction.R = os.Stdin
	instruction.W = os.Stdout
	instruction.E = os.Stderr
	instruction.State = false
	return instruction
}

func (ins *Instruction) Append(arg string) {
	if ins.Cmd.Path == "" {
		path, err := exec.LookPath(arg)
		if err == nil {
			ins.Cmd.Path = path
		} else {
			ins.Cmd.Path = arg
		}
		ins.Cmd.Args = []string{arg}
	} else {
		ins.Cmd.Args = append(ins.Cmd.Args, arg)
	}
}

func (ins *Instruction) PipeRead(r *os.File) {
	ins.Cmd.Stdin = r
	ins.R = r
}

func (ins *Instruction) PipeWrite(w *os.File) {
	ins.Cmd.Stdout = w
	ins.W = w
}

func (ins *Instruction) IsChdir() bool {
	return ins.Cmd.Path == "cd"
}

func (ins *Instruction) Chdir() bool {
	cmd := ins.Cmd
	if len(cmd.Args) > 2 {
		fmt.Println("Too many arguments for cd:", cmd.Args)
		return false
	} else {
		if err := os.Chdir(cmd.Args[1]); err != nil {
			fmt.Println(err.Error())
			return false
		}
	}
	return true
}

type Instructions []*Instruction

func (inss *Instructions) Append(ins *Instruction) {
	*inss = append(*inss, ins)
}

type InstructionExec struct {
	insType InstructionType
	cmd     *exec.Cmd
}

type InstructionPipe struct {
	insType InstructionType
	cmd     *exec.Cmd
	r, w    *os.File
}

func Tokenize(s *string) []string {
	var tokens []string
	var toParse bool
	var from int

	toParse = true
	from = 0

	ignore := strings.Split(" \t", "")
	noParse := strings.Split("\"`'", "")
	d := strings.Split("|><&", "")
	for i, c := range *s {
		// TODO:
		// Implement this using a stack
		if slices.Contains(noParse, string(c)) {
			toParse = !toParse
			continue
		}

		if !toParse {
			continue
		}

		if slices.Contains(ignore, string(c)) {
			if i == from {
				from++
				continue
			}
			tokens = append(tokens, (*s)[from:i])
			from = i + 1
			tokens[len(tokens)-1] = os.ExpandEnv(tokens[len(tokens)-1])
		} else if slices.Contains(d, string(c)) {
			if c == '&' {
				if i+2 < len(*s) && (*s)[i+1] == '&' {
					if i == from {
						tokens = append(tokens, "&&")
					} else {
						tokens = append(tokens, (*s)[from:i], "&&")
						tokens[len(tokens)-2] = os.ExpandEnv(tokens[len(tokens)-2])
					}
					from = i + 2
				}
				continue
			}
			if i == from {
				tokens = append(tokens, string(c))
			} else {
				tokens = append(tokens, (*s)[from:i], string(c))
				tokens[len(tokens)-2] = os.ExpandEnv(tokens[len(tokens)-2])
			}
			from = i + 1
		}

	}

	if from != len(*s) {
		tokens = append(tokens, (*s)[from:])
		tokens[len(tokens)-1] = os.ExpandEnv(tokens[len(tokens)-1])
	}

	return tokens
}

func parseToken(instruction *Instruction, tokens []string) int {
	var i int
	var err error

	for i = 0; i < len(tokens); i++ {
		token := tokens[i]
		if token == "|" {
			instruction.InsType = PIPE
			return i
		} else if token == "&&" {
			if i != 0 {
				return i - 1
			}
			instruction.InsType = WAIT
			return i
		} else if token == ">" {
			if i == len(tokens) {
				fmt.Println("Expected string after >")
				return -1
			}
			instruction.W, err = os.Create(tokens[i+1])
			if err != nil {
				panic(err)
			}
			instruction.Cmd.Stdout = instruction.W
			i++
		} else if token == "<" {
			if i == len(tokens) {
				fmt.Println("Expected string after <")
				return -1
			}
			instruction.R, err = os.Open(tokens[i+1])
			if err != nil {
				panic(err)
			}
			instruction.Cmd.Stdin = instruction.R
			i++
		} else {
			instruction.Append(token)
		}
	}
	return i
}

func Parse(tokens []string) Instructions {
	var instructions Instructions
	var instruction *Instruction

	begin := 0
	for i := 0; i < len(tokens) && begin < len(tokens); i++ {
		instruction = NewInstruction("")
		i = parseToken(instruction, tokens[begin:])
		if i < 0 {
			return instructions
		}
		instructions.Append(instruction)
		begin += i + 1
	}

	return instructions
}

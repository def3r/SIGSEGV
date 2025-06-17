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
	R, W    *os.File
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
	instruction.State = false
	return instruction
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

func Parse(tokens []string) Instructions {
	var instructions Instructions
	var instruction *Instruction

	begin := 0
	for i, token := range tokens {
		if token == "|" {
			instruction = NewInstruction(tokens[begin], tokens[begin+1:i]...)
			instruction.InsType = PIPE
			instructions.Append(instruction)
			begin = i + 1
		} else if token == "&&" {
			instruction = NewInstruction(tokens[begin], tokens[begin+1:i]...)
			instructions.Append(instruction)
			instruction = NewInstruction("", "")
			instruction.InsType = WAIT
			instructions.Append(instruction)
			begin = i + 1
		}
	}
	instruction = NewInstruction(tokens[begin], tokens[begin+1:]...)
	instructions.Append(instruction)

	return instructions
}

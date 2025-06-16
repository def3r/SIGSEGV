package main

import (
	"bufio"
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
	insType InstructionType
	cmd     *exec.Cmd
	r, w    *os.File
	state   bool
}

func newInstruction(path string, args ...string) *Instruction {
	execCmd := exec.Command(path, args...)
	execCmd.Stdout = os.Stdout
	execCmd.Stdin = os.Stdin
	execCmd.Stderr = os.Stderr

	instruction := new(Instruction)
	instruction.insType = EXEC
	instruction.cmd = execCmd
	instruction.r = os.Stdin
	instruction.w = os.Stdout
	instruction.state = false
	return instruction
}

func (ins *Instruction) PipeRead(r *os.File) {
	ins.cmd.Stdin = r
	ins.r = r
}

func (ins *Instruction) PipeWrite(w *os.File) {
	ins.cmd.Stdout = w
	ins.w = w
}

func (ins *Instruction) isChdir() bool {
	return ins.cmd.Path == "cd"
}

func (ins *Instruction) Chdir() bool {
	cmd := ins.cmd
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

func (inss *Instructions) append(ins *Instruction) {
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

type DLSH struct {
	piped        bool
	r            *os.File
	w            *os.File
	instructions Instructions
	err          error
	ins          *Instruction
}

func newDLSH() *DLSH {
	dlsh := new(DLSH)
	dlsh.piped = false
	dlsh.r = os.Stdin
	dlsh.w = os.Stdout
	return dlsh
}

func (dlsh *DLSH) ExecPipe() {
	ins := dlsh.ins
	dlsh.piped = true
	ins.PipeRead(dlsh.r)
	dlsh.r, dlsh.w, dlsh.err = os.Pipe()
	if dlsh.err != nil {
		fmt.Println(dlsh.err.Error())
		os.Exit(1)
	}
	ins.PipeWrite(dlsh.w)
	if ins.isChdir() {
		return
	}
	if dlsh.err = ins.cmd.Start(); dlsh.err != nil {
		fmt.Println(dlsh.err.Error())
		return
	} else {
		ins.state = true
	}
}

func (dlsh *DLSH) DrainPipeline() {
	for i, ins := range dlsh.instructions {
		if ins.state {
			if err := ins.cmd.Wait(); err != nil {
				fmt.Println(err.Error())
			}
			ins.state = false
		} else if !ins.isChdir() {
			continue
		}
		if i > 0 && dlsh.instructions[i-1].insType == PIPE {
			ins.r.Close()
		}
		if ins.insType == PIPE {
			ins.w.Close()
		}
	}
}

func (dlsh *DLSH) DrainExec() {
	ins := dlsh.ins
	dlsh.piped = false
	ins.PipeRead(dlsh.r)
	dlsh.r = os.Stdin
	dlsh.w = os.Stdout
	if dlsh.err = ins.cmd.Start(); dlsh.err == nil {
		ins.state = true
		dlsh.DrainPipeline()
	} else {
		fmt.Println(dlsh.err.Error())
	}
}

func main() {
	fmt.Println("$HOME=" + os.Getenv("HOME"))

	scanner := bufio.NewScanner(os.Stdin)

	for {
		cwd, err := os.Getwd()
		if err != nil {
			fmt.Println("Failed to get current dir")
			os.Exit(1)
		}
		fmt.Print(cwd + "> ")
		if !scanner.Scan() {
			fmt.Println("Failed to scan")
			break
		}

		line := scanner.Text()
		line = strings.Trim(line, " \t")
		tokens := Tokenize(&line)
		fmt.Println(tokens, len(tokens))
		if line == "exit" {
			break
		}

		dlsh := newDLSH()
		dlsh.instructions = Parse(tokens)
		for _, ins := range dlsh.instructions {
			dlsh.ins = ins
			cmd := ins.cmd
			if cmd.Path == "cd" {
				if !ins.Chdir() {
					break
				}
				if ins.insType != PIPE {
					continue
				}
			}

			switch ins.insType {
			case EXEC:
				if dlsh.piped {
					dlsh.DrainExec()
				} else {
					if err := cmd.Run(); err != nil {
						fmt.Println(err.Error())
					}
				}
			case PIPE:
				dlsh.ExecPipe()
			case WAIT:
				dlsh.DrainPipeline()
			}

			if dlsh.err != nil {
				break
			}
		}
	}
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
			instruction = newInstruction(tokens[begin], tokens[begin+1:i]...)
			instruction.insType = PIPE
			instructions.append(instruction)
			begin = i + 1
		} else if token == "&&" {
			instruction = newInstruction(tokens[begin], tokens[begin+1:i]...)
			instructions.append(instruction)
			instruction = newInstruction("", "")
			instruction.insType = WAIT
			instructions.append(instruction)
			begin = i + 1
		}
	}
	instruction = newInstruction(tokens[begin], tokens[begin+1:]...)
	instructions.append(instruction)

	return instructions
}

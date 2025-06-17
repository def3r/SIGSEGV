package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"

	cl "dlsh/utils/cmdline"
	eu "dlsh/utils/execunit"
)

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
		tokens := cl.Tokenize(&line)
		fmt.Println(tokens, len(tokens))
		if line == "exit" {
			break
		}

		dlsh := eu.NewExecUnit()
		dlsh.Instructions = cl.Parse(tokens)
		for _, ins := range dlsh.Instructions {
			dlsh.Ins = ins
			cmd := ins.Cmd
			if cmd.Path == "cd" {
				if !ins.Chdir() {
					break
				}
				if ins.InsType != cl.PIPE {
					continue
				}
			}

			switch ins.InsType {
			case cl.EXEC:
				if dlsh.Piped {
					dlsh.DrainExec()
				} else {
					if err := cmd.Run(); err != nil {
						fmt.Println(err.Error())
					}
				}
			case cl.PIPE:
				dlsh.ExecPipe()
			case cl.WAIT:
				dlsh.DrainPipeline()
			}

			if dlsh.Err != nil {
				break
			}
		}
	}
}

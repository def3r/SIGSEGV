package execunit

import (
	cl "dlsh/utils/cmdline"
	"fmt"
	"os"
)

type ExecUnit struct {
	Piped        bool
	R            *os.File
	W            *os.File
	Instructions cl.Instructions
	Err          error
	Ins          *cl.Instruction
}

func NewExecUnit() *ExecUnit {
	dlsh := new(ExecUnit)
	dlsh.Piped = false
	dlsh.R = os.Stdin
	dlsh.W = os.Stdout
	return dlsh
}

func (dlsh *ExecUnit) ExecPipe() {
	ins := dlsh.Ins
	dlsh.Piped = true
	ins.PipeRead(dlsh.R)
	dlsh.R, dlsh.W, dlsh.Err = os.Pipe()
	if dlsh.Err != nil {
		fmt.Println(dlsh.Err.Error())
		os.Exit(1)
	}
	ins.PipeWrite(dlsh.W)
	if ins.IsChdir() {
		return
	}
	if dlsh.Err = ins.Cmd.Start(); dlsh.Err != nil {
		fmt.Println(dlsh.Err.Error())
		return
	} else {
		ins.State = true
	}
}

func (dlsh *ExecUnit) DrainPipeline() {
	for i, ins := range dlsh.Instructions {
		if ins.State {
			if err := ins.Cmd.Wait(); err != nil {
				fmt.Println(err.Error())
			}
			ins.State = false
		} else if !ins.IsChdir() {
			continue
		}
		if i > 0 && dlsh.Instructions[i-1].InsType == cl.PIPE {
			ins.R.Close()
		}
		if ins.InsType == cl.PIPE {
			ins.W.Close()
		}
	}
}

func (dlsh *ExecUnit) DrainExec() {
	ins := dlsh.Ins
	dlsh.Piped = false
	ins.PipeRead(dlsh.R)
	dlsh.R = os.Stdin
	dlsh.W = os.Stdout
	if dlsh.Err = ins.Cmd.Start(); dlsh.Err == nil {
		ins.State = true
		dlsh.DrainPipeline()
	} else {
		fmt.Println(dlsh.Err.Error())
	}
}

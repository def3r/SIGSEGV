using LLVM
using JSON

function run_c2q(val1::Int, val2::Int)::String
    payload = Dict(
        "family" => "ADD",
        "instance" => Dict(
            "operands" => [val1, val2],
            "bits" => 32
        )
    )

    json_path = "add_$(val1)_$(val2).json"
    qasm_path = "add_$(val1)_$(val2)_circuit.qasm"
    c2q_bin   = "/home/def3r/iitk/C2Q/.venv/bin/c2q-json"

    open(json_path, "w") do f
        JSON.print(f, payload, 4)
    end

    ret = run(`$c2q_bin --input $json_path --export`, wait=true)
    if !success(ret)
      error("c2q-json failed for $val1 + $val2")
    end

    println("Generated QASM: $qasm_path")
    return qasm_path
end

context!(Context()) do
    mod = parse(LLVM.Module, String(read("after_mem2reg.ll")))

    # declare quantum_add as external function in the IR
    # i32 quantum_add(i32, i32)
    ctx = context(mod)
    i32 = LLVM.Int32Type()
    quantum_add_type = LLVM.FunctionType(i32, [i32, i32])
    quantum_add_fn = LLVM.Function(mod, "quantum_add", quantum_add_type)

    instructions_to_replace = []

    for func in functions(mod)
        for bb in blocks(func)
            for inst in instructions(bb)
                if opcode(inst) == LLVM.API.LLVMAdd
                    op1 = operands(inst)[1]
                    op2 = operands(inst)[2]

                    if isa(op1, ConstantInt) && isa(op2, ConstantInt)
                        val1 = convert(Int, op1)
                        val2 = convert(Int, op2)
                        push!(instructions_to_replace, (inst, val1, val2))
                    end
                end
            end
        end
    end

    # do replacements outside the iteration loop
    for (inst, val1, val2) in instructions_to_replace
        println("Replacing add $val1 + $val2 with call @quantum_add")

        qasm_path = run_c2q(val1, val2)

        LLVM.@dispose builder=IRBuilder() begin
            position!(builder, inst)
            call = call!(builder, quantum_add_type, quantum_add_fn,
                         [operands(inst)[1], operands(inst)[2]], "")
            replace_uses!(inst, call)
        end

        unsafe_delete!(LLVM.parent(inst), inst)
    end

    # write the mutated IR
    open("after_quantum_pass.ll", "w") do f
        write(f, string(mod))
    end

    println("Written mutated IR to after_quantum_pass.ll")
end

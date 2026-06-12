using LLVM
using JSON

# ── config ────────────────────────────────────────────────────────────────────

const C2Q_BIN    = get(ENV, "C2Q_BIN", "/home/def3r/iitk/C2Q/.venv/bin/c2q-json")
const INPUT_LL   = "after_mem2reg.ll"
const OUTPUT_LL  = "after_quantum_pass.ll"
const TABLE_C    = "qtable.c"
const LINK_SH    = "link.sh"

# ── c2q helpers ───────────────────────────────────────────────────────────────

function generate_qasm(val1::Int, val2::Int)::Tuple{String, String}
    key       = "add_$(val1)_$(val2)"
    json_path = "$(key).json"
    qasm_path = "$(key)_circuit.qasm"

    # skip if already generated
    if isfile(qasm_path)
        println("[pass] reusing existing QASM: $qasm_path")
        return key, qasm_path
    end

    payload = Dict(
        "family"   => "ADD",
        "instance" => Dict(
            "operands" => [val1, val2],
            "bits"     => 32
        )
    )

    open(json_path, "w") do f
        JSON.print(f, payload, 4)
    end

    proc = run(`$C2Q_BIN --input $json_path --export`, wait=true)
    if !success(proc)
        error("c2q-json failed for $val1 + $val2")
    end

    println("[pass] generated QASM: $qasm_path")
    return key, qasm_path
end

# ── emit qtable.c ────────────────────────────────────────────────────────────
# uses linker symbols from: ld -r -b binary <key>_circuit.qasm -o <key>.o
# no hex dump — raw bytes embedded by linker, zero size inflation

function emit_table(entries::Vector{Tuple{String, String}})
    open(TABLE_C, "w") do f
        println(f, "#include \"libqrun.h\"")
        println(f, "#include <stddef.h>")
        println(f, "")

        # forward declare linker symbols for each qasm file
        for (key, _) in entries
            sym = "$(key)_circuit_qasm"
            println(f, "extern char _binary_$(sym)_start[];")
        end
        println(f, "")
        # ld also generates _size symbols — use them directly, no constructor needed
        for (key, _) in entries
            sym = "$(key)_circuit_qasm"
            println(f, "extern char   _binary_$(sym)_size[];")
        end

        println(f, "")
        println(f, "quantum_entry_t quantum_table[] = {")
        for (key, _) in entries
            sym = "$(key)_circuit_qasm"
            println(f, "    { \"$(key)\", _binary_$(sym)_start, (size_t)_binary_$(sym)_size },")
        end
        println(f, "};")
        println(f, "")
        println(f, "int quantum_table_size = $(length(entries));")
    end

    println("[pass] emitted $TABLE_C with $(length(entries)) entries (linker symbols)")
end

# ── embed qasm files via ld ───────────────────────────────────────────────────

function embed_qasm(entries::Vector{Tuple{String, String}})
    for (key, qasm_path) in entries
        obj_path = "$(key).o"
        proc = run(`ld -r -b binary $qasm_path -o $obj_path`, wait=true)
        if !success(proc)
            error("[pass] ld failed for $qasm_path")
        end
        println("[pass] embedded $qasm_path → $obj_path")
    end
end

# ── emit link.sh ─────────────────────────────────────────────────────────────

function emit_link_script(entries::Vector{Tuple{String, String}})
    open(LINK_SH, "w") do f
        println(f, "#!/bin/bash")
        println(f, "set -e")
        println(f, "")
        println(f, "llc -filetype=obj --relocation-model=pic $OUTPUT_LL -o program.o")
        println(f, "clang -fPIE -c libqrun.c -o libqrun.o")
        println(f, "clang -fPIE -c $TABLE_C  -o qtable.o")
        println(f, "")
        objs = ["program.o", "libqrun.o", "qtable.o", "qworker.o"]
        for (key, _) in entries
            push!(objs, "$(key).o")
        end
        println(f, "clang -fPIE $(join(objs, " ")) -o qprog")
        println(f, "echo 'build complete → ./qprog'")
    end
    run(`chmod +x $LINK_SH`)
    println("[pass] emitted $LINK_SH")
end

# ── main pass ─────────────────────────────────────────────────────────────────

context!(Context()) do
    mod = parse(LLVM.Module, String(read(INPUT_LL)))

    # declare quantum_execute(i8*, i8*) -> i32
    i32      = LLVM.Int32Type()
    i8ptr    = LLVM.PointerType(LLVM.Int8Type())
    qe_type  = LLVM.FunctionType(i32, [i8ptr, i8ptr])
    qe_fn    = LLVM.Function(mod, "quantum_execute", qe_type)

    # collect constant add instructions
    to_replace = Vector{Tuple{LLVM.Instruction, Int, Int}}()

    for func in functions(mod)
        for bb in blocks(func)
            for inst in instructions(bb)
                if opcode(inst) == LLVM.API.LLVMAdd
                    op1 = operands(inst)[1]
                    op2 = operands(inst)[2]
                    if isa(op1, ConstantInt) && isa(op2, ConstantInt)
                        val1 = convert(Int, op1)
                        val2 = convert(Int, op2)
                        push!(to_replace, (inst, val1, val2))
                    end
                end
            end
        end
    end

    # deduplicate: track which (val1, val2) pairs we have already processed
    seen    = Set{Tuple{Int,Int}}()
    entries = Vector{Tuple{String, String}}()

    for (_, val1, val2) in to_replace
        key = (val1, val2)
        if key ∉ seen
            push!(seen, key)
            qasm_key, qasm_path = generate_qasm(val1, val2)
            push!(entries, (qasm_key, qasm_path))
        end
    end

    # emit table + embed qasm objects + link script
    emit_table(entries)
    embed_qasm(entries)
    emit_link_script(entries)

    # replace instructions
    for (inst, val1, val2) in to_replace
        println("[pass] replacing add $val1 + $val2 → call @quantum_execute")

        key_str     = "add_$(val1)_$(val2)"
        decoder_str = "add"

        LLVM.@dispose builder=IRBuilder() begin
            position!(builder, inst)

            # create global string constants for key and decoder
            key_gv     = globalstring_ptr!(builder, key_str,     "qkey")
            decoder_gv = globalstring_ptr!(builder, decoder_str, "qdecoder")

            call = call!(builder, qe_type, qe_fn,
                        [key_gv, decoder_gv], "")

            replace_uses!(inst, call)
        end

        unsafe_delete!(LLVM.parent(inst), inst)
    end

    # write mutated IR
    open(OUTPUT_LL, "w") do f
        write(f, string(mod))
    end

    println("[pass] wrote mutated IR → $OUTPUT_LL")
    println("[pass] done. $(length(to_replace)) add(s) replaced.")
end

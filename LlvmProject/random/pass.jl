using LLVM
using JSON

# config
const C2Q_BIN  = get(ENV, "C2Q_BIN", "/home/def3r/iitk/C2Q/.venv/bin/c2q-json")
const INPUT_LL = isempty(ARGS) ? error("usage: julia pass.jl <input.ll>") : ARGS[1]
const OUTPUT_LL = "after_quantum_pass.ll"
const TABLE_C   = "qtable.c"
const LINK_SH   = "link.sh"

# c2q helpers

function generate_qasm_arithmetic(op::String, family::String, bits::Int, val1::Int, val2::Int)::Tuple{String,String}
    key       = "$(op)_$(val1)_$(val2)"
    json_path = "$(key).json"
    qasm_path = "$(key)_circuit.qasm"

    if isfile(qasm_path)
        println("[pass] reusing existing QASM: $qasm_path")
        return key, qasm_path
    end

    payload = Dict(
        "family"   => family,
        "instance" => Dict(
            "operands" => [val1, val2],
            "bits"     => bits
        )
    )

    open(json_path, "w") do f
        JSON.print(f, payload, 4)
    end

    proc = run(`$C2Q_BIN --input $json_path --export`, wait=true)
    if !success(proc)
        error("c2q-json failed for $val1 $op $val2")
    end

    println("[pass] generated QASM: $qasm_path")
    return key, qasm_path
end

function generate_qasm_maxcut(key::String, edges::Vector{Tuple{Int,Int}})::Tuple{String,String}
    json_path = "$(key).json"
    qasm_path = "$(key)_qaoa.qasm"

    if isfile(qasm_path)
        println("[pass] reusing existing QASM: $qasm_path")
        return key, qasm_path
    end

    payload = Dict(
        "family" => "MaxCut",
        "instance" => Dict(
            "graph_rep" => "edge_list",
            "graphs"    => Dict(
                "G1" => [[u, v] for (u, v) in edges]
            )
        )
    )

    open(json_path, "w") do f
        JSON.print(f, payload, 4)
    end

    proc = run(`$C2Q_BIN --input $json_path --export`, wait=true)
    if !success(proc)
        error("c2q-json failed for maxcut $key")
    end

    println("[pass] generated QASM: $qasm_path")
    return key, qasm_path
end


# emit edge table into qtable.c (appended after quantum_table)
function emit_edge_table(f::IO, edge_entries::Dict{String, Vector{Tuple{Int,Int}}})
    println(f, "")
    for (key, edges) in edge_entries
        varname = replace(key, "-" => "_")
        row_strs = ["{$(u),$(v)}" for (u, v) in edges]
        println(f, "static const int $(varname)_raw_edges[][2] = {$(join(row_strs, ","))};")
    end
    println(f, "")
    println(f, "quantum_edge_entry_t quantum_edge_table[] = {")
    if isempty(edge_entries)
        println(f, "    { NULL, NULL, 0 }")
    else
        for (key, edges) in edge_entries
            varname = replace(key, "-" => "_")
            println(f, "    { \"$(key)\", $(varname)_raw_edges, $(length(edges)) },")
        end
    end
    println(f, "};")
    println(f, "int quantum_edge_table_size = $(length(edge_entries));")
end

# emit qtable.c
function emit_table(entries::Vector{Tuple{String,String}}, edge_entries::Dict{String, Vector{Tuple{Int,Int}}})
    open(TABLE_C, "w") do f
        println(f, "#include \"libqrun.h\"")
        println(f, "#include <stddef.h>")
        println(f, "")

        for (key, qasm_path) in entries
            sym = replace(basename(qasm_path), r"[^a-zA-Z0-9_]" => "_")
            println(f, "extern char _binary_$(sym)_start[];")
        end
        println(f, "")
        for (key, qasm_path) in entries
            sym = replace(basename(qasm_path), r"[^a-zA-Z0-9_]" => "_")
            println(f, "extern char _binary_$(sym)_size[];")
        end

        println(f, "")
        println(f, "quantum_entry_t quantum_table[] = {")
        for (key, qasm_path) in entries
            sym = replace(basename(qasm_path), r"[^a-zA-Z0-9_]" => "_")
            println(f, "    { \"$(key)\", _binary_$(sym)_start, (size_t)_binary_$(sym)_size },")
        end
        println(f, "};")
        println(f, "")
        println(f, "int quantum_table_size = $(length(entries));")
        emit_edge_table(f, edge_entries)
    end
    println("[pass] emitted $TABLE_C with $(length(entries)) entries")
end

# embed qasm files via ld

function embed_qasm(entries::Vector{Tuple{String,String}})
    for (key, qasm_path) in entries
        obj_path = "$(key).o"
        proc = run(`ld -r -b binary $qasm_path -o $obj_path`, wait=true)
        if !success(proc)
            error("[pass] ld failed for $qasm_path")
        end
        println("[pass] embedded $qasm_path → $obj_path")
    end
end

# emit link.sh

function emit_link_script(entries::Vector{Tuple{String,String}})
    objs = ["program.o", "libqrun.o", "qtable.o", "qworker.o"]
    for (key, _) in entries
        push!(objs, "$(key).o")
    end

    open(LINK_SH, "w") do f
        println(f, "#!/bin/bash")
        println(f, "set -e")
        println(f, "")
        println(f, "KEEP=0")
        println(f, "for arg in \"\$@\"; do")
        println(f, "    case \"\$arg\" in")
        println(f, "        --keep|-k) KEEP=1 ;;")
        println(f, "    esac")
        println(f, "done")
        println(f, "")
        println(f, "llc -filetype=obj --relocation-model=pic $OUTPUT_LL -o program.o")
        println(f, "clang -fPIE -c libqrun.c -o libqrun.o")
        println(f, "clang -fPIE -c $TABLE_C  -o qtable.o")
        println(f, "")
        println(f, "clang -fPIE $(join(objs, " ")) -o qprog")
        println(f, "echo 'build complete → ./qprog'")
        println(f, "")
        jsons = join(["$(key).json" for (key, _) in entries], " ")
        qasms = join([qasm_path for (_, qasm_path) in entries], " ")
        println(f, "if [ \"\$KEEP\" -eq 0 ]; then")
        println(f, "    rm -f $(join(objs, " "))")
        println(f, "    rm -f $jsons $qasms")
        println(f, "    rm -f $INPUT_LL $OUTPUT_LL")
        println(f, "    echo 'artifacts cleaned up (use --keep to preserve)'")
        println(f, "fi")
    end
    run(`chmod +x $LINK_SH`)
    println("[pass] emitted $LINK_SH")
end

# replace instruction with quantum_execute call

function replace_with_quantum_execute!(
        inst::LLVM.Instruction,
        key::String,
        decoder::String,
        qe_type::LLVM.FunctionType,
        qe_fn::LLVM.Function)

    LLVM.@dispose builder=IRBuilder() begin
        position!(builder, inst)
        key_gv     = globalstring_ptr!(builder, key,     "qkey")
        decoder_gv = globalstring_ptr!(builder, decoder, "qdecoder")
        call       = call!(builder, qe_type, qe_fn, [key_gv, decoder_gv], "")
        replace_uses!(inst, call)
    end
    unsafe_delete!(LLVM.parent(inst), inst)
end

# main pass

context!(Context()) do
    ir_str = String(read(INPUT_LL))
    ir_str = replace(ir_str, r"\bcaptures\(none\)" => "nocapture")
    mod = parse(LLVM.Module, ir_str)

    # declare quantum_execute(i8*, i8*) -> i32
    i32     = LLVM.Int32Type()
    i8ptr   = LLVM.PointerType(LLVM.Int8Type())
    qe_type = LLVM.FunctionType(i32, [i8ptr, i8ptr])
    qe_fn   = LLVM.Function(mod, "quantum_execute", qe_type)

    entries      = Vector{Tuple{String,String}}()
    edge_entries = Dict{String, Vector{Tuple{Int,Int}}}()

    # pass 1: constant arithmetic (add, mul)

    to_replace_add = Vector{Tuple{LLVM.Instruction,Int,Int}}()
    to_replace_mul = Vector{Tuple{LLVM.Instruction,Int,Int}}()

    for func in functions(mod)
        for bb in blocks(func)
            for inst in instructions(bb)
                if opcode(inst) == LLVM.API.LLVMAdd
                    op1, op2 = operands(inst)[1], operands(inst)[2]
                    if isa(op1, ConstantInt) && isa(op2, ConstantInt)
                        push!(to_replace_add, (inst, convert(Int, op1), convert(Int, op2)))
                    end
                elseif opcode(inst) == LLVM.API.LLVMMul
                    op1, op2 = operands(inst)[1], operands(inst)[2]
                    if isa(op1, ConstantInt) && isa(op2, ConstantInt)
                        push!(to_replace_mul, (inst, convert(Int, op1), convert(Int, op2)))
                    end
                end
            end
        end
    end

    seen_add = Set{Tuple{Int,Int}}()
    seen_mul = Set{Tuple{Int,Int}}()

    for (_, v1, v2) in to_replace_add
        (v1, v2) ∉ seen_add && (push!(seen_add, (v1,v2)); push!(entries, generate_qasm_arithmetic("add","ADD",32,v1,v2)))
    end
    for (_, v1, v2) in to_replace_mul
        (v1, v2) ∉ seen_mul && (push!(seen_mul, (v1,v2)); push!(entries, generate_qasm_arithmetic("mul","MUL",8,v1,v2)))
    end

    # pass 2: maxcut - read AST analysis from quantum_manifest.json (written by clang.jl)

    maxcut_calls = Vector{Tuple{LLVM.Instruction,String}}()

    manifest = isfile("quantum_manifest.json") ? JSON.parsefile("quantum_manifest.json") :
               Dict{String,Any}("maxcut" => [])

    seen_graphs = Dict{Vector{Tuple{Int,Int}}, String}()

    for entry in get(manifest, "maxcut", [])
        func_name = entry["function_name"]
        edges     = Tuple{Int,Int}[(e[1], e[2]) for e in entry["edges"]]

        # deduplicate by graph structure
        key = if haskey(seen_graphs, edges)
            seen_graphs[edges]
        else
            k = "maxcut_$(length(seen_graphs)+1)"
            seen_graphs[edges] = k
            edge_entries[k] = edges
            push!(entries, generate_qasm_maxcut(k, edges))
            k
        end

        # find every call instruction in the IR that calls func_name
        for func in functions(mod)
            for bb in blocks(func)
                for inst in instructions(bb)
                    opcode(inst) != LLVM.API.LLVMCall && continue
                    ops = collect(operands(inst))
                    isempty(ops) && continue
                    callee = ops[end]
                    isa(callee, LLVM.Function) || continue
                    LLVM.name(callee) == func_name || continue
                    println("[pass] replacing call to $func_name → @quantum_execute($key)")
                    push!(maxcut_calls, (inst, key))
                end
            end
        end
    end

    # emit artifacts

    emit_table(entries, edge_entries)
    embed_qasm(entries)
    emit_link_script(entries)

    # replace arithmetic instructions

    for (inst, v1, v2) in to_replace_add
        println("[pass] replacing add $v1 + $v2 → @quantum_execute")
        replace_with_quantum_execute!(inst, "add_$(v1)_$(v2)", "add", qe_type, qe_fn)
    end

    for (inst, v1, v2) in to_replace_mul
        println("[pass] replacing mul $v1 × $v2 → @quantum_execute")
        replace_with_quantum_execute!(inst, "mul_$(v1)_$(v2)", "mul", qe_type, qe_fn)
    end

    # replace maxcut calls

    for (call_inst, key) in maxcut_calls
        println("[pass] replacing maxcut call → @quantum_execute($key)")
        replace_with_quantum_execute!(call_inst, key, "maxcut", qe_type, qe_fn)
    end

    # write mutated IR

    open(OUTPUT_LL, "w") do f
        write(f, string(mod))
    end

    println("[pass] wrote mutated IR → $OUTPUT_LL")
    println("[pass] done.")
    println("       $(length(to_replace_add)) add(s)")
    println("       $(length(to_replace_mul)) mul(s)")
    println("       $(length(maxcut_calls)) maxcut(s)")
end

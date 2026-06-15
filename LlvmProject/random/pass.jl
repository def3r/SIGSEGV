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

# maxcut detection
#
# targets mem2reg IR — canonical edge list MaxCut pattern:
#
#   loop header:
#     phi %.acc = [0, entry], [%.acc_new, latch]   ← cut accumulator
#     phi %.idx = [0, entry], [%.idx+1, latch]     ← loop index i
#     icmp slt %.idx, num_edges
#
#   loop body:
#     GEP [2 x i32], ptr edges, %.idx              ← edges[i]
#     GEP [2 x i32], ptr edges[i], 0, 0            ← u = edges[i][0]
#     GEP [2 x i32], ptr edges[i], 0, 1            ← v = edges[i][1]
#     GEP i32, ptr partition, u                    ← partition[u]
#     GEP i32, ptr partition, v                    ← partition[v]
#     icmp ne partition[u], partition[v]           ← key signature
#
#   conditional:
#     add %.acc, 1                                 ← cut++
#     phi selecting cut++ or cut
#
# returns: (edges, num_edges_arg, call_site) or nothing

# helper: is this instruction a GEP with [2 x i32] element type?
function is_edge_gep(inst::LLVM.Instruction)::Bool
    opcode(inst) != LLVM.API.LLVMGetElementPtr && return false
    # check the source element type is [2 x i32]
    # GEP on [2 x i32] will have type [2 x i32]*
    ty = LLVM.llvmtype(inst)
    return true  # we verify structure via operand patterns below
end

# helper: trace a value back through sext to find the underlying value
function strip_sext(v::LLVM.Value)::LLVM.Value
    if isa(v, LLVM.Instruction) && opcode(v) == LLVM.API.LLVMSExt
        return operands(v)[1]
    end
    return v
end

# detect maxcut in a single function
# returns (edges::Vector{Tuple{Int,Int}}, func) or nothing
function detect_maxcut(func::LLVM.Function)
    bbs = collect(blocks(func))
    length(bbs) < 3 && return nothing

    # function must have exactly 3 params: (ptr edges, i32 num_edges, ptr partition)
    params = collect(parameters(func))
    length(params) != 3 && return nothing

    edges_param     = params[1]  # ptr to [2 x i32]
    num_edges_param = params[2]  # i32
    partition_param = params[3]  # ptr to i32

    # find loop header: bb with two phi nodes and icmp slt
    loop_header = nothing
    acc_phi     = nothing
    idx_phi     = nothing

    for bb in bbs
        insts = collect(instructions(bb))
        phis  = filter(i -> opcode(i) == LLVM.API.LLVMPHI, insts)
        length(phis) != 2 && continue

        # find icmp slt %.idx, num_edges_param
        icmps = filter(i -> opcode(i) == LLVM.API.LLVMICmp, insts)
        isempty(icmps) && continue

        has_bound_check = any(icmps) do icmp
            ops = collect(operands(icmp))
            length(ops) >= 2 && ops[2] == num_edges_param
        end
        !has_bound_check && continue

        # identify accumulator phi (starts at 0, incremented by 1)
        # and index phi (starts at 0, incremented by 1)
        p1, p2 = phis[1], phis[2]

        # both should start at 0
        function phi_starts_at_zero(phi)
            for i in 1:length(collect(LLVM.incoming(phi)))
                val, _ = collect(LLVM.incoming(phi))[i]
                isa(val, ConstantInt) && convert(Int, val) == 0 && return true
            end
            return false
        end

        (!phi_starts_at_zero(p1) || !phi_starts_at_zero(p2)) && continue

        loop_header = bb
        # we'll identify acc vs idx more precisely later
        acc_phi = p1
        idx_phi = p2
        break
    end

    loop_header === nothing && return nothing

    # find loop body: bb with the edge GEP pattern
    loop_body = nothing
    u_val     = nothing
    v_val     = nothing
    cut_icmp  = nothing

    for bb in bbs
        bb == loop_header && continue
        insts = collect(instructions(bb))

        # look for two GEPs on edges_param (the [2 x i32] array)
        edge_geps = filter(insts) do i
            opcode(i) != LLVM.API.LLVMGetElementPtr && return false
            ops = collect(operands(i))
            isempty(ops) && return false
            # base pointer should trace back to edges_param
            base = ops[1]
            return base == edges_param || (isa(base, LLVM.Instruction) &&
                   opcode(base) == LLVM.API.LLVMGetElementPtr &&
                   collect(operands(base))[1] == edges_param)
        end

        length(edge_geps) < 2 && continue

        # look for icmp ne — the key MaxCut signature
        icmps = filter(i -> opcode(i) == LLVM.API.LLVMICmp, insts)
        isempty(icmps) && continue

        ne_icmp = findfirst(icmps) do icmp
            ops = collect(operands(icmp))
            length(ops) >= 2 || return false
            # both operands should be loads from partition_param GEPs
            op1 = ops[1]
            op2 = ops[2]
            function is_partition_load(v)
                !isa(v, LLVM.Instruction) && return false
                opcode(v) != LLVM.API.LLVMLoad && return false
                ptr = collect(operands(v))[1]
                !isa(ptr, LLVM.Instruction) && return false
                opcode(ptr) != LLVM.API.LLVMGetElementPtr && return false
                collect(operands(ptr))[1] == partition_param
            end
            is_partition_load(op1) && is_partition_load(op2)
        end

        ne_icmp === nothing && continue

        cut_icmp  = icmps[ne_icmp]
        loop_body = bb

        # extract u and v from the partition GEP indices
        icmp_ops = collect(operands(cut_icmp))
        load1    = icmp_ops[1]
        load2    = icmp_ops[2]

        gep1 = collect(operands(load1))[1]
        gep2 = collect(operands(load2))[1]

        # u and v are the indices into partition — strip sext
        u_val = strip_sext(collect(operands(gep1))[2])
        v_val = strip_sext(collect(operands(gep2))[2])
        break
    end

    loop_body === nothing && return nothing
    println("[pass] found MaxCut pattern in function: $(LLVM.name(func))")

    # find call site in main/caller
    # look for a call to this function anywhere in the module
    call_inst = nothing
    for f in LLVM.functions(LLVM.parent(func))
        for bb in blocks(f)
            for inst in instructions(bb)
                opcode(inst) != LLVM.API.LLVMCall && continue
                ops = collect(operands(inst))
                isempty(ops) && continue
                # last operand of call is the callee
                if ops[end] == func
                    call_inst = inst
                    break
                end
            end
            call_inst !== nothing && break
        end
        call_inst !== nothing && break
    end

    call_inst === nothing && return nothing

    # extract graph from constant data at call site
    # look for @__const.*edges* global in the module
    call_ops = collect(operands(call_inst))
    # call_ops[1] = edges ptr, call_ops[2] = num_edges, call_ops[3] = partition ptr

    length(call_ops) < 4 && return nothing

    edges_arg = call_ops[1]
    num_edges_arg = call_ops[2]

    # try to extract num_edges as a constant
    !isa(num_edges_arg, ConstantInt) && return nothing
    num_edges = convert(Int, num_edges_arg)

    # trace edges_arg back to a global constant
    # handles two patterns:
    #   GEP @global, 0, 0  →  direct
    #   GEP (alloca memcpy'd from @global), 0, 0  →  mem2reg pattern
    function trace_to_global(v::LLVM.Value)
        isa(v, LLVM.GlobalVariable) && return v
        isa(v, LLVM.Instruction) || return nothing
        if opcode(v) == LLVM.API.LLVMGetElementPtr
            return trace_to_global(collect(operands(v))[1])
        end
        if opcode(v) == LLVM.API.LLVMAlloca
            for use in LLVM.uses(v)
                user = LLVM.user(use)
                isa(user, LLVM.Instruction) || continue
                opcode(user) != LLVM.API.LLVMCall && continue
                ops = collect(operands(user))
                length(ops) < 3 && continue
                callee = ops[end]
                isa(callee, LLVM.Function) || continue
                !startswith(LLVM.name(callee), "llvm.memcpy") && continue
                ops[1] == v || continue   # v must be the dst
                return trace_to_global(ops[2])
            end
        end
        return nothing
    end

    global_var = trace_to_global(edges_arg)
    global_var === nothing && return nothing

    # read the initializer of the global — it's [N x [2 x i32]]
    init = LLVM.initializer(global_var)
    init === nothing && return nothing

    # extract edges from the constant array
    # operands() returns Value[] for ConstantDataArray (packed int data), so use
    # LLVMGetAggregateElement which works for all aggregate constant kinds
    edges = Vector{Tuple{Int,Int}}()
    try
        for i in 0:num_edges-1
            edge = LLVM.Value(LLVM.API.LLVMGetAggregateElement(init, UInt32(i)))
            u    = convert(Int, LLVM.Value(LLVM.API.LLVMGetAggregateElement(edge, UInt32(0))))
            v    = convert(Int, LLVM.Value(LLVM.API.LLVMGetAggregateElement(edge, UInt32(1))))
            push!(edges, (u, v))
        end
    catch e
        println("[pass] could not extract graph constants: $e")
        return nothing
    end

    println("[pass] extracted graph: $num_edges edges → $edges")
    return (edges, call_inst)
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

    # pass 2: maxcut pattern detection

    maxcut_calls = Vector{Tuple{LLVM.Instruction,String}}()  # (call_inst, key)
    seen_graphs  = Dict{Vector{Tuple{Int,Int}}, String}()    # edges → key

    for func in functions(mod)
        result = detect_maxcut(func)
        result === nothing && continue

        edges, call_inst = result

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

        push!(maxcut_calls, (call_inst, key))
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

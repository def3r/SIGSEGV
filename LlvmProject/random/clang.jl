using Clang.LibClang
using JSON

if isempty(ARGS)
    println("usage: julia clang.jl <input.c>")
    exit(0)
end
const INPUT_C = ARGS[1]
isfile(INPUT_C) || error("file not found: $INPUT_C")

# string helpers
function cu_spelling(c::CXCursor)::String
    s = clang_getCursorSpelling(c)
    r = unsafe_string(clang_getCString(s))
    clang_disposeString(s)
    r
end

function cu_type_spelling(c::CXCursor)::String
    t = clang_getCursorType(c)
    s = clang_getTypeSpelling(t)
    r = unsafe_string(clang_getCString(s))
    clang_disposeString(s)
    r
end

cu_kind(c::CXCursor) = clang_getCursorKind(c)

# cursor tree

function cu_children(parent::CXCursor)::Vector{CXCursor}
    result = CXCursor[]
    fn = let r = result
        (c::CXCursor, _::CXCursor, _::Ptr{Cvoid}) -> (push!(r, c); CXChildVisit_Continue)
    end
    GC.@preserve result begin
        clang_visitChildren(parent,
            @cfunction($fn, CXChildVisitResult, (CXCursor, CXCursor, Ptr{Cvoid})),
            C_NULL)
    end
    result
end

function cu_walk(c::CXCursor, fn::Function)
    fn(c)
    for ch in cu_children(c)
        cu_walk(ch, fn)
    end
end

function cu_find(c::CXCursor, pred::Function)::Union{CXCursor,Nothing}
    for ch in cu_children(c)
        pred(ch) && return ch
        found = cu_find(ch, pred)
        found !== nothing && return found
    end
    nothing
end

cu_find_kind(c, k) = cu_find(c, x -> cu_kind(x) == k)
cu_find_kind_self(c, k) = cu_kind(c) == k ? c : cu_find_kind(c, k)

# tokenization

function cu_tokens(tu, c::CXCursor)::Vector{String}
    ext = clang_getCursorExtent(c)
    tptr = Ref{Ptr{CXToken}}(C_NULL)
    nref = Ref{Cuint}(0)
    clang_tokenize(tu, ext, tptr, nref)
    n = Int(nref[])
    ptr = tptr[]
    strs = String[]
    for i in 0:n-1
        tok = unsafe_load(ptr, i + 1)
        s = clang_getTokenSpelling(tu, tok)
        push!(strs, unsafe_string(clang_getCString(s)))
        clang_disposeString(s)
    end
    n > 0 && clang_disposeTokens(tu, ptr, Cuint(n))
    strs
end

# decl-reference check

function refs_decl(c::CXCursor, decl::CXCursor)::Bool
    found = Ref(false)
    cu_walk(c, x -> begin
        found[] && return
        cu_kind(x) == CXCursor_DeclRefExpr &&
            clang_equalCursors(clang_getCursorReferenced(x), decl) != 0 &&
            (found[] = true)
    end)
    found[]
end

# integer literal evaluation

function eval_intlit(c::CXCursor)::Union{Int,Nothing}
    ev = clang_Cursor_Evaluate(c)
    ev == C_NULL && return nothing
    v = Int(clang_EvalResult_getAsInt(ev))
    clang_EvalResult_dispose(ev)
    v
end

# ---- loop pattern helpers ----

# VarDecl for the index variable declared in a ForStmt's init clause (int i = 0).
function find_loop_var(for_stmt::CXCursor)::Union{CXCursor,Nothing}
    ch = cu_children(for_stmt)
    isempty(ch) && return nothing
    init = ch[1]
    cu_kind(init) == CXCursor_DeclStmt || return nothing
    vars = cu_children(init)
    isempty(vars) && return nothing
    first = vars[1]
    cu_kind(first) == CXCursor_VarDecl ? first : nothing
end

# Walk c's subtree for a DeclRefExpr, but stop (don't recurse) at ArraySubscriptExpr
# boundaries so we don't accidentally find a sub-array's base.
# Returns the referenced declaration cursor, or nothing.
function shallow_declref_decl(c::CXCursor)::Union{CXCursor,Nothing}
    cu_kind(c) == CXCursor_ArraySubscriptExpr && return nothing
    cu_kind(c) == CXCursor_DeclRefExpr && return clang_getCursorReferenced(c)
    for ch in cu_children(c)
        r = shallow_declref_decl(ch)
        r !== nothing && return r
    end
    nothing
end

# Declaration of the outermost array in an ArraySubscriptExpr without descending
# into nested subscripts on the base side.
# e.g.  partition[u]          → ParmDecl(partition)
#       partition[edges[i][0]] → ParmDecl(partition)
function array_base_decl(sub::CXCursor)::Union{CXCursor,Nothing}
    ch = cu_children(sub)
    isempty(ch) && return nothing
    shallow_declref_decl(ch[1])
end

# Second child (index expression) of an ArraySubscriptExpr, or nothing.
function array_index_expr(sub::CXCursor)::Union{CXCursor,Nothing}
    ch = cu_children(sub)
    length(ch) >= 2 ? ch[2] : nothing
end

# Check whether outer_sub has the shape  arr2d[loop_var][0_or_1].
# Returns (arr2d_declaration_cursor, col) or nothing.
function check_2d_array_access(outer_sub::CXCursor, loop_var::CXCursor)::Union{Tuple{CXCursor,Int},Nothing}
    och = cu_children(outer_sub)
    length(och) < 2 && return nothing

    # Outer index: must be an integer literal 0 or 1.
    col = Ref{Union{Int,Nothing}}(nothing)
    cu_walk(och[2], c -> begin
        col[] !== nothing && return
        cu_kind(c) == CXCursor_IntegerLiteral || return
        v = eval_intlit(c)
        (v == 0 || v == 1) && (col[] = v)
    end)
    col[] === nothing && return nothing

    # Base of outer: must be an inner ArraySubscriptExpr arr2d[loop_var].
    inner = cu_find_kind_self(och[1], CXCursor_ArraySubscriptExpr)
    inner === nothing && return nothing
    ich = cu_children(inner)
    length(ich) < 2 && return nothing

    refs_decl(ich[2], loop_var) || return nothing   # inner index references the loop var

    arr_decl = shallow_declref_decl(ich[1])         # inner base is the edges array
    arr_decl === nothing && return nothing

    (arr_decl, col[])
end

# Given the index expression idx of partition[idx], determine whether it resolves
# to an edge-list access arr2d[loop_var][0_or_1], handling two surface forms:
#
#   Pattern A (explicit vars):   idx = DeclRefExpr(u)  where  int u = arr2d[i][0]
#   Pattern B (inline):          idx = ArraySubscriptExpr(arr2d[i][0])
#
# Returns (edges_array_decl, col) or nothing.
function resolve_to_edge_access(idx::CXCursor, loop_var::CXCursor)::Union{Tuple{CXCursor,Int},Nothing}
    # Pattern B: idx is (or wraps) an ArraySubscriptExpr directly.
    sub = cu_find_kind_self(idx, CXCursor_ArraySubscriptExpr)
    if sub !== nothing
        r = check_2d_array_access(sub, loop_var)
        r !== nothing && return r
    end

    # Pattern A: idx is a reference to a local VarDecl whose initializer is arr2d[i][col].
    ref_decl = shallow_declref_decl(idx)
    ref_decl === nothing && return nothing
    cu_kind(ref_decl) == CXCursor_VarDecl || return nothing
    init_sub = cu_find_kind(ref_decl, CXCursor_ArraySubscriptExpr)
    init_sub === nothing && return nothing
    check_2d_array_access(init_sub, loop_var)
end

# Check whether for_stmt is a MaxCut cut-counting loop, matching:
#
#   Pattern A:  for(...) { int u=edges[i][0]; int v=edges[i][1];
#                          if (partition[u]  != partition[v])  cut++; }
#   Pattern B:  for(...) { if (partition[edges[i][0]] != partition[edges[i][1]]) cut++; }
#
# Returns the edges array declaration (VarDecl or ParmDecl) on success, nothing otherwise.
function check_maxcut_loop(for_stmt::CXCursor, tu)::Union{CXCursor,Nothing}
    loop_var = find_loop_var(for_stmt)
    loop_var === nothing && return nothing

    for_ch = cu_children(for_stmt)
    length(for_ch) < 2 && return nothing
    body = for_ch[end]      # body is always the last child of ForStmt

    if_stmt = cu_find_kind_self(body, CXCursor_IfStmt)
    if_stmt === nothing && return nothing

    if_ch = cu_children(if_stmt)
    length(if_ch) < 2 && return nothing
    cond = if_ch[1]
    then_body = if_ch[2]

    # Condition: BinaryOperator '!='
    binop = cu_find_kind_self(cond, CXCursor_BinaryOperator)
    binop === nothing && return nothing
    "!=" ∉ cu_tokens(tu, binop) && return nothing

    bch = cu_children(binop)
    length(bch) < 2 && return nothing
    lhs_expr, rhs_expr = bch[1], bch[2]

    # Both operands must be subscripts of the SAME array (the partition array).
    lhs_sub = cu_find_kind_self(lhs_expr, CXCursor_ArraySubscriptExpr)
    rhs_sub = cu_find_kind_self(rhs_expr, CXCursor_ArraySubscriptExpr)
    (lhs_sub === nothing || rhs_sub === nothing) && return nothing

    lhs_base = array_base_decl(lhs_sub)
    rhs_base = array_base_decl(rhs_sub)
    (lhs_base === nothing || rhs_base === nothing) && return nothing
    clang_equalCursors(lhs_base, rhs_base) == 0 && return nothing   # same partition array

    # The subscript indices must resolve to edges[loop_var][0] and edges[loop_var][1].
    lhs_idx = array_index_expr(lhs_sub)
    rhs_idx = array_index_expr(rhs_sub)
    (lhs_idx === nothing || rhs_idx === nothing) && return nothing

    lhs_access = resolve_to_edge_access(lhs_idx, loop_var)
    rhs_access = resolve_to_edge_access(rhs_idx, loop_var)
    (lhs_access === nothing || rhs_access === nothing) && return nothing

    lhs_arr, lhs_col = lhs_access
    rhs_arr, rhs_col = rhs_access
    clang_equalCursors(lhs_arr, rhs_arr) == 0 && return nothing     # same edges array
    Set([lhs_col, rhs_col]) != Set([0, 1]) && return nothing    # one [0] and one [1]

    # Then-body must contain a ++ (the cut accumulator).
    unary = cu_find_kind_self(then_body, CXCursor_UnaryOperator)
    unary === nothing && return nothing
    "++" ∉ cu_tokens(tu, unary) && return nothing

    lhs_arr   # return the edges array declaration
end

# ---- edge list extraction from call site ----

function extract_edges(var_decl::CXCursor)::Union{Vector{Tuple{Int,Int}},Nothing}
    outer = nothing
    for ch in cu_children(var_decl)
        cu_kind(ch) == CXCursor_InitListExpr && (outer = ch; break)
    end
    outer === nothing && return nothing

    edges = Tuple{Int,Int}[]
    for inner in cu_children(outer)
        cu_kind(inner) != CXCursor_InitListExpr && continue
        ints = Int[]
        cu_walk(inner, c -> begin
            if cu_kind(c) == CXCursor_IntegerLiteral
                v = eval_intlit(c)
                v !== nothing && push!(ints, v)
            end
        end)
        length(ints) >= 2 && push!(edges, (ints[1], ints[2]))
    end

    isempty(edges) ? nothing : edges
end

# ---- main detection ----

function detect_maxcut(root::CXCursor, tu)::Vector{Dict{String,Any}}
    results = Dict{String,Any}[]

    # Functions where edges arrive as a parameter: (func_decl, func_name, param_idx_1based)
    param_funcs = Vector{Tuple{CXCursor,String,Int}}()

    for func in cu_children(root)
        cu_kind(func) != CXCursor_FunctionDecl && continue
        clang_isCursorDefinition(func) == 0 && continue

        func_name = cu_spelling(func)
        params = filter(ch -> cu_kind(ch) == CXCursor_ParmDecl, cu_children(func))

        cu_walk(func, c -> begin
            cu_kind(c) != CXCursor_ForStmt && return

            edges_decl = check_maxcut_loop(c, tu)
            edges_decl === nothing && return

            println("[clang.jl] structural match in: $func_name  (edges: $(cu_spelling(edges_decl)))")

            if cu_kind(edges_decl) == CXCursor_ParmDecl
                # Edges passed as a parameter — resolve at call sites (Pass 2).
                pidx = findfirst(p -> clang_equalCursors(p, edges_decl) != 0, params)
                pidx === nothing && return
                # Register this function once (first matching loop wins).
                any(t -> clang_equalCursors(t[1], func) != 0, param_funcs) && return
                push!(param_funcs, (func, func_name, pidx))

            elseif cu_kind(edges_decl) == CXCursor_VarDecl
                # Edges defined locally — can extract directly, but there is no call
                # to replace in the IR.  Emit the data anyway; pass.jl will skip it
                # if it finds no call site named func_name.
                edges = extract_edges(edges_decl)
                edges === nothing && return
                println("[clang.jl] local edges in $func_name: $(length(edges)) → $edges")
                push!(results, Dict{String,Any}(
                    "function_name" => func_name,
                    "edges" => [[u, v] for (u, v) in edges],
                    "num_edges" => length(edges)
                ))
            end
        end)
    end

    # Pass 2: find CallExprs invoking each param-based function; extract the edge arg.
    if !isempty(param_funcs)
        cu_walk(root, c -> begin
            cu_kind(c) != CXCursor_CallExpr && return

            callee_ref = cu_find(c, ch -> cu_kind(ch) == CXCursor_DeclRefExpr)
            callee_ref === nothing && return
            referenced = clang_getCursorReferenced(callee_ref)

            match_idx = findfirst(t -> clang_equalCursors(t[1], referenced) != 0, param_funcs)
            match_idx === nothing && return

            _, func_name, pidx = param_funcs[match_idx]

            clang_Cursor_getNumArguments(c) < pidx && return
            arg = clang_Cursor_getArgument(c, pidx - 1)   # 0-indexed

            var_ref = cu_find(arg, ch -> cu_kind(ch) == CXCursor_DeclRefExpr)
            var_ref === nothing && return

            var_decl = clang_getCursorReferenced(var_ref)
            cu_kind(var_decl) != CXCursor_VarDecl && return
            !occursin("[2]", cu_type_spelling(var_decl)) && return

            edges = extract_edges(var_decl)
            edges === nothing && return

            println("[clang.jl] call to $func_name: $(length(edges)) edges → $edges")
            push!(results, Dict{String,Any}(
                "function_name" => func_name,
                "edges" => [[u, v] for (u, v) in edges],
                "num_edges" => length(edges)
            ))
        end)
    end

    results
end

# main
idx = clang_createIndex(0, 0)
tu = clang_parseTranslationUnit(idx, INPUT_C, C_NULL, 0, C_NULL, 0, CXTranslationUnit_None)
tu == C_NULL && (@error "parse failed: $INPUT_C"; exit(1))

root = clang_getTranslationUnitCursor(tu)
detections = detect_maxcut(root, tu)

open("quantum_manifest.json", "w") do f
    JSON.print(f, Dict{String,Any}("maxcut" => detections), 2)
end

n = length(detections)
println("[clang.jl] quantum_manifest.json - $n detection$(n == 1 ? "" : "s")")

ccall(:_exit, Cvoid, (Cint,), 0)

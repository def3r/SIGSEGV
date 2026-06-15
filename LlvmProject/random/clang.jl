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

# Direct (non-recursive) children of a cursor.
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

# Depth-first walk of the subtree rooted at cursor (including cursor itself).
function cu_walk(c::CXCursor, fn::Function)
    fn(c)
    for ch in cu_children(c)
        cu_walk(ch, fn)
    end
end

# First descendant (depth-first) that satisfies pred, or nothing.
function cu_find(c::CXCursor, pred::Function)::Union{CXCursor, Nothing}
    for ch in cu_children(c)
        pred(ch) && return ch
        found = cu_find(ch, pred)
        found !== nothing && return found
    end
    nothing
end

# First descendant of the given kind, or nothing.
cu_find_kind(c, k) = cu_find(c, x -> cu_kind(x) == k)

# Cursor itself if it matches, otherwise first descendant that does.
cu_find_kind_self(c, k) = cu_kind(c) == k ? c : cu_find_kind(c, k)

# tokenization

# All token spellings covering the cursor's source extent.
# Used to check operator tokens ('!=', '++') that libclang doesn't expose directly.
function cu_tokens(tu, c::CXCursor)::Vector{String}
    ext  = clang_getCursorExtent(c)
    tptr = Ref{Ptr{CXToken}}(C_NULL)
    nref = Ref{Cuint}(0)
    clang_tokenize(tu, ext, tptr, nref)
    n   = Int(nref[])
    ptr = tptr[]
    strs = String[]
    for i in 0:n-1
        tok = unsafe_load(ptr, i + 1)
        s   = clang_getTokenSpelling(tu, tok)
        push!(strs, unsafe_string(clang_getCString(s)))
        clang_disposeString(s)
    end
    n > 0 && clang_disposeTokens(tu, ptr, Cuint(n))
    strs
end

# decl-reference check

# True if any DeclRefExpr in the subtree rooted at cursor references decl.
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

function eval_intlit(c::CXCursor)::Union{Int, Nothing}
    ev = clang_Cursor_Evaluate(c)
    ev == C_NULL && return nothing
    v = Int(clang_EvalResult_getAsInt(ev))
    clang_EvalResult_dispose(ev)
    v
end

# structural pattern checks

# 1. FunctionDecl with exactly 3 params: (int (*)[2], int, int *)
function check_signature(func::CXCursor)::Union{Vector{CXCursor}, Nothing}
    cu_kind(func) != CXCursor_FunctionDecl && return nothing
    clang_isCursorDefinition(func) == 0    && return nothing

    params = filter(cu_children(func)) do ch
        cu_kind(ch) == CXCursor_ParmDecl
    end
    length(params) != 3 && return nothing

    !occursin("[2]", cu_type_spelling(params[1])) && return nothing  # int (*)[2]
    cu_type_spelling(params[2]) != "int"          && return nothing  # int num_edges
    cu_type_spelling(params[3]) != "int *"        && return nothing  # int *partition

    params
end

# 2. ForStmt in the function body whose condition (a BinaryOperator child of the
#    ForStmt) references num_edges_param.  Returns the ForStmt cursor, or nothing.
function check_for_loop(func::CXCursor, num_edges_param::CXCursor)::Union{CXCursor, Nothing}
    for_stmt = cu_find_kind(func, CXCursor_ForStmt)
    for_stmt === nothing && return nothing

    for ch in cu_children(for_stmt)
        cu_kind(ch) == CXCursor_BinaryOperator || continue
        refs_decl(ch, num_edges_param) && return for_stmt
    end
    nothing
end

# 3. Loop body contains VarDecls of the form:
#       int u = edges[i][0];   (outer index == 0)
#       int v = edges[i][1];   (outer index == 1)
#    Verified by:  outer subscript → inner subscript referencing edges_param
#                  outer index     → IntegerLiteral 0 or 1
function check_edge_vardecls(body::CXCursor, edges_param::CXCursor)::Bool
    has_u = Ref(false)
    has_v = Ref(false)

    cu_walk(body, (c) -> begin
        cu_kind(c) != CXCursor_VarDecl && return

        # Outermost ArraySubscriptExpr inside this VarDecl's initializer
        outer = cu_find_kind(c, CXCursor_ArraySubscriptExpr)
        outer === nothing && return

        och = cu_children(outer)
        length(och) < 2 && return

        # Outer index: IntegerLiteral 0 or 1
        idx = nothing
        cu_walk(och[2], x -> begin
            idx !== nothing && return
            if cu_kind(x) == CXCursor_IntegerLiteral
                v = eval_intlit(x)
                (v == 0 || v == 1) && (idx = v)
            end
        end)
        idx === nothing && return

        # Base of outer subscript must be (or contain) an ArraySubscriptExpr
        # that references edges_param - this is the inner edges[i] subscript
        inner = cu_find_kind_self(och[1], CXCursor_ArraySubscriptExpr)
        inner === nothing && return
        refs_decl(inner, edges_param) || return

        idx == 0 && (has_u[] = true)
        idx == 1 && (has_v[] = true)
    end)

    has_u[] && has_v[]
end

# 4+5. Loop body contains an IfStmt whose:
#   condition: BinaryOperator '!=' where both operands reference partition_param
#   body:      UnaryOperator '++'  (the cut accumulator increment)
function check_if_stmt(body::CXCursor, partition_param::CXCursor, tu)::Bool
    if_stmt = cu_find_kind(body, CXCursor_IfStmt)
    if_stmt === nothing && return false

    if_ch = cu_children(if_stmt)
    length(if_ch) < 2 && return false

    cond      = if_ch[1]
    then_body = if_ch[2]

    # Condition must be (or contain) a BinaryOperator '!='
    binop = cu_find_kind_self(cond, CXCursor_BinaryOperator)
    binop === nothing && return false

    "!=" ∉ cu_tokens(tu, binop) && return false

    bch = cu_children(binop)
    length(bch) < 2 && return false

    # Both operands must reference partition_param (partition[u] and partition[v])
    refs_decl(bch[1], partition_param) || return false
    refs_decl(bch[2], partition_param) || return false

    # Then-body must contain a UnaryOperator '++'
    unary = cu_find_kind_self(then_body, CXCursor_UnaryOperator)
    unary === nothing && return false

    "++" ∉ cu_tokens(tu, unary) && return false

    true
end

# Full structural check combining all five criteria.
function is_maxcut_pattern(func::CXCursor, tu)::Bool
    params = check_signature(func)
    params === nothing && return false

    edges_param, num_edges_param, partition_param = params

    for_stmt = check_for_loop(func, num_edges_param)
    for_stmt === nothing && return false

    body = cu_children(for_stmt)[end]  # body is always the last child of ForStmt

    check_edge_vardecls(body, edges_param) || return false
    check_if_stmt(body, partition_param, tu) || return false

    true
end

# edge list extraction from call site

# Given a VarDecl of type int[N][2] with a constant initializer, extract edges.
function extract_edges(var_decl::CXCursor)::Union{Vector{Tuple{Int,Int}}, Nothing}
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

# main detection

function detect_maxcut(root::CXCursor, tu)::Vector{Dict{String,Any}}
    results = Dict{String,Any}[]

    # Pass 1: find all FunctionDecl definitions matching the structural pattern.
    maxcut_funcs = CXCursor[]
    cu_walk(root, c -> begin
        is_maxcut_pattern(c, tu) || return
        push!(maxcut_funcs, c)
        params = filter(cu_children(c)) do ch; cu_kind(ch) == CXCursor_ParmDecl; end
        println("[clang.jl] structural match: $(cu_spelling(c))",
                "(", join(cu_type_spelling.(params), ", "), ")")
    end)

    isempty(maxcut_funcs) && return results

    # Pass 2: find CallExprs invoking those functions and extract edge data.
    cu_walk(root, c -> begin
        cu_kind(c) != CXCursor_CallExpr && return

        # The first DeclRefExpr descendent is the callee reference.
        callee_ref = cu_find(c, ch -> cu_kind(ch) == CXCursor_DeclRefExpr)
        callee_ref === nothing && return

        referenced = clang_getCursorReferenced(callee_ref)
        match_idx  = findfirst(f -> clang_equalCursors(f, referenced) != 0, maxcut_funcs)
        match_idx === nothing && return

        func_name = cu_spelling(maxcut_funcs[match_idx])

        clang_Cursor_getNumArguments(c) < 1 && return
        arg0 = clang_Cursor_getArgument(c, 0)

        # arg0 is ArrayToPointerDecay → DeclRefExpr → VarDecl with int[N][2] type
        var_ref = cu_find(arg0, ch -> cu_kind(ch) == CXCursor_DeclRefExpr)
        var_ref === nothing && return

        var_decl = clang_getCursorReferenced(var_ref)
        cu_kind(var_decl) != CXCursor_VarDecl && return

        !occursin("[2]", cu_type_spelling(var_decl)) && return

        edges = extract_edges(var_decl)
        edges === nothing && return

        println("[clang.jl] call to $func_name: $(length(edges)) edges → $edges")
        push!(results, Dict{String,Any}(
            "function_name" => func_name,
            "edges"         => [[u, v] for (u, v) in edges],
            "num_edges"     => length(edges)
        ))
    end)

    results
end

# main
idx = clang_createIndex(0, 0)
tu  = clang_parseTranslationUnit(idx, INPUT_C, C_NULL, 0, C_NULL, 0, CXTranslationUnit_None)
tu == C_NULL && (@error "parse failed: $INPUT_C"; exit(1))

root       = clang_getTranslationUnitCursor(tu)
detections = detect_maxcut(root, tu)

open("quantum_manifest.json", "w") do f
    JSON.print(f, Dict{String,Any}("maxcut" => detections), 2)
end

n = length(detections)
println("[clang.jl] quantum_manifest.json - $n detection$(n == 1 ? "" : "s")")

# Skip dispose + bypass Julia atexit handlers - the OS reclaims everything.
# clang_disposeTranslationUnit + exit(0) races with @cfunction GC teardown.
ccall(:_exit, Cvoid, (Cint,), 0)

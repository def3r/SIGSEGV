Dialect -> Set of new operations

[Resrc](https://gemini.google.com/u/2/app/d8517f891f79de17) # MLIR
- Work on MLIR began with a realization that modern machine learning frameworks
  are composed ofmany different compilers, graph technologies, and runtime
  systems (see Figure 1)—which did notshare a common infrastructure or design
  point, and not all of which were following best practicesin compiler design
- We soon realized that the compiler industry as a whole has a similar problem:
  existing systems likeLLVM are very successful at unifying and integrating
  work across a range of different languageimplementations, but modern high
  level languages often end up building their own high-level IR andreinventing
  a lot of the same kinds of technology for higher levels of abstractio

## Design Principles
- Little builtin, everything customizable
  - The system is based on a minimal number of fundamen-tal concepts, leaving
    most of the intermediate representation fully customizable.
  - types, operations and attributes, which are the most common in IRs—should
    be used toexpress everything else

- SSA and regions [Region
  Explanation](https://gemini.google.com/u/2/app/d8517f891f79de17#:~:text=In%20traditional%20compiler%20design%2C%20a%20region)
  - While many existing IRs use a flat, linearized CFG,representing higher
    level abstractions push introducing nested regions as a first-class concept
    in theIR. This goes beyond the traditional region formation to lift higher
    level abstractions (e.g., loop trees),speeding up the compilation process
    or extracting instruction, or SIMD parallelism
  - To support heterogeneous compilation, the system has to support the
    expression of structured controlflow, concurrency constructs, closures in
    source languages, and many other purposes. One specificchallenge is to make
    CFG-based analyses and transformations compose over nested regions.

- Progressive lowering
  - support progressive lowering
  - i.e. from the higher-levelrepresentation down to the lowest-level, with the
    lowering being performed in small steps alongmultiple abstraction levels.
  - compiler passes can be roughly categorized into four roles: (1) optimizing
    transformations,(2) enabling transformations, (3) lowering and (4) cleanup.
  - The system should allow for mixing and matching these roles at the
    granularity of a single operation rather than sequencing passes on the full
    compilation unit

- Maintain higher-level semantics
  - Attempts to raise semanticsonce lowered are fragile and shoehorning this
    information into a low-level often invasive
  - Instead, the system should maintain structure of computation and
    progressively lower to the hardwareabstraction.
  - For example, the system should preserve thestructured control flow such as
    loop structure throughout the relevant transformations; removing
    thisstructure, i.e. going to CFG-based control flow, essentially means no
    further transformations will beperformed on this level.
  - As a corollary, mixing different levels of abstractions and different
    concepts in the same IR is a keyproperty of the system to allow a part of
    the representation to remain in higher-level abstractionwhile another part
    is lowered.
  - This would enable, for instance, a compiler for a custom accelerator
    toreuse some higher-level structure and abstractions defined by the system
    alongside with primitivescalar/vector instructions specific to the
    accelerator.

- IR validation
  [Explanation](https://gemini.google.com/u/2/app/d8517f891f79de17#:~:text=In%20MLIR%2C%20because%20it%20is%20designed%20to%20be%20an%20%22extensible%22%20system%2C%20the%20standard%20compiler%20rules%20you%20learned%20at%20Winter%20School%20get%20pushed%20to%20their%20limits)
  - The openness of the ecosystem calls for an extensive validation mechanism.
  - While verification and testing are useful to detect compiler bugs, and to
    capture IR invariants, the needfor robust validation methodologies and
    tools is amplified in an extensible system.
  - The mechanism should aim to make this easy to define and as declarative as
    practical, providing a single source oftruth.

- Declarative rewrite patterns
  - a compiler infrastructure is only as good as the transformations it
    supports
  - Common transformations should be implementable as rewrite rules expressed
    declaratively, in a machine-analyzable format to reason about properties of
    the rewrites such as complexity and completion.

- Source location tracking and traceability
  - This intendsto address the lack-of-transparency problem, common to complex
    compilation systems, where it isvirtually impossible to understand how the
    final representation was constructed from the original one.
  - This is particularly problematic when compiling safety-critical and
    sensitive applications, wheretracing lowering and optimization steps is an
    essential component of software certification procedures
  - Optimizations may alter or completely invalidate such protections; thislack
    of transparency is known as WYSINWYX(What You See Is Not What You Execute)
    in secure compilation
  - One indirect goal ofaccurately propagating high-level information to the
    lower levels is to help support secure andtraceable compilation

## IR Design Details
- Operations
  - The unit of semantics in MLIR is an “operation”, referred to as Op
  - Everything from“instruction” to “function” to “module” are modeled as Ops
  - Ops take and produce zero or more values, called operands and results
    respectively, andthese are maintained in SSA form
  - Ops may also have Attributes, Regions, Block Arguments, and
    LocationInformation as well

- Attributes
  - MLIR attribute is structured compile-time static information

- Location information
  - provides a compact representation for location information
  - used to keep the source program stack trace that produced an Op, to
    generate debug information
  - Location information is also extensible

- Regions and blocks
  - region providesthe mechanism for nested structure in MLIR
  - a region contains a list of blocks, and a block containsa list of
    operations (which may contain regions)
  - the blocks inside the region (if more thanone) form a Control Flow Graph
    (CFG)
  - each block ends with a terminator operation, that mayhave successor blocks
    to which the control flow may be transferred
  - Instead of using φ nodes, MLIR uses a functional form of SSA where
    terminators pass valuesinto block arguments defined by the successor block

- Value dominance and visibility
  - Ops can only use values that are in scope, i.e. visible accordingto SSA
    dominance, nesting, and semantic restrictions imposed by enclosing
    operations
  - Region-based visibility is defined based on simple nesting of regions: if
    the operand to an Op isoutside the current region, then it must be defined
    lexically above and outside the region of the use.
  - MLIR also allows operations to be defined as isolated from above,
    indicating that the operation isa scope barrier—e.g. the “std.func” Op
    defines a function, and it is not valid for operations withinthe function
    to refer to values defined outside the function
  - module containing isolated-from-above Ops may be processed in parallel by
    an MLIRcompiler since no use-def chains may cross the isolation barriers

- Symbols and symbol tables
  - The IR does notprescribe what symbols are used for, leaving it up to the Op
    definition
  - Symbols are most usefulfor named entities need not obey SSA: they cannot be
    redefined within the same table, but theycan be used prior to their
    definition
  - Without this mechanism, it would have been impossible to define,e.g.,
    recursive function referring to themselves in their definition
  - Symbol tables can be nested if anOp with a symbol table attached has
    associated regions containing similar Ops.

- Dialects
  - provide a logical grouping of Ops,attributes and types under a unique
    namespace
  - do not introduce any newsemantics but serve as a logical grouping mechanism
    and can be used to provide dialect generic Opsupport
  - each Op, type and attribute belongs to exactly one dialect, MLIRexplicitly
    supports a mix of dialects to enable progressive lowering

- Type system
  - Types provide compile-time semantics forthe IR
  - MLIR enforces strict type equality checking and does notprovide type
    conversion rules
  - MLIR only supports non-dependent types, including trivial,parametric,
    function, sum and product types
  - possible to implement a dependent typesystem by combining Ops with symbols
    and user-defined types such types will be opaque to the IR

- Standard types
  - MLIR provides a standardized set of commonly used types, includingarbitrary
    precision integers, standard floating point types, and simple common
    containers—tuples,multi-dimensional vectors, and tensors

- Functions and modules
  - A module is an Op with a single region containing a single block, and
    terminated by a dummy Opthat does not transfer the control flow
  - module defines a symbol and can be referenced.
  - Like anyblock, its body contains a list of Ops, which may be functions,
    global variables, compiler metadata,or other top-level constructs
  - A function is an Op with a single region, with arguments corresponding to
    function arguments
  - defines a symbol and can be referenced by name.
  - A“return” terminator does not have successors and instead terminates the
    region execution, transferringthe control flow back to the call-site of the
    function

## IR Infrastructure

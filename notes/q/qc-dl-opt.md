[Paper](https://arxiv.org/pdf/2104.05573)

# AI Powered Compiler Techniques for DL CodeOptimization
- Multiple con-siderations including multi-level cache hierarchy, and wideSIMD
  units of CPU platforms influence the choice of pro-gram transformations to
  apply for performance optimization.
- machine learning powered compilertechniques to optimize loop nests
- the current state-of-practice isto use expert-coded high performance
  libraries such as InteloneDNN in deep learning frameworks such as Tensor-Flow
  and PyTorch to achieve good performance
  - being reliant on libraries for performance is not scalable.
-  an attractive alternative solution is to developcompilation techniques that
   automate code optimization andachieve similar performance levels as
   expert-coded libraries.
- High level optimizations perform loop optimizationssuch as loop reordering
  and tiling to derive a loop struc-ture that utilizes the cache hierarchy of
  the computer sys-tem to the fullest extent possible
- Low level optimizationsgenerate vector code using the target machine’s
  intrinsics;reinforcement learning methodology guides the derivationof high
  performance vector code
- We evaluate our automated compiler system on GEMMs (GEneral Matrix
  Multiplications) which lie at the heart of deep learning

## THE COMPILER OPTIMIZATIONWORKFLOW
- The highlevel optimizer first optimizes the loop structure and thenpasses on
  the code to the low level optimer.
- high level optimizer uses polyhedral compilation tech-niques for optimization
  of the loop structure to take advan-tage of the multi-level caches of the CPU
  platform
- oopreordering and tiling transformations are applied and thebest loop order
  and tile sizes are determined by the highlevel optimizer
- nhance data locality – both the spa-tial and temporal locality
- 95% of the deep learning infer-ence workloads (MLPs, CNNs, and LSTMs) can be
  formulatedin terms of matrix-multiplication

## HIGH-LEVEL POLYHEDRAL LOOPOPTIMIZATIONS
- The exact data dependences inloop nests can be computed in the polyhedral
  model and areexpressed as maps from source iterations to target
  iterationsinvolved in the dependence
- we consider four kinds of dependences
  - Read-After-Read (RAR)
  - Read-After-Write (RAW, a.k.a flow)
  - Write-After-Read (WAR, a.k.a anti)
  - Write-After-Write(WAW)
- The data footprint of the loop can be computed by applyingread and write
  relations on the iteration space set: 𝑟1(𝐼) ∪ 𝑟2(𝐼) ∪ 𝑟3 (𝐼) ∪ 𝑤1 (𝐼) {For
  GEMMs}

### Loop transformations
- We create a number of code variants by applying loop re-ordering and tiling
  transformations and using the PolyDLtechniques select the top code variants.

- *Working set size computation*
  - perform cache data reuseanalysis to characterize a loop-nest’s behavior
    with respectto a given cache hierarchy
  - for the input cachehierarchy determines which data reuses are exploitable
    atvarious levels of cache
  - Each data dependence in a loop is alsoan instance of data reuse – the
    source and target iterationsinvolved in the dependence touch the same data
    elementand therefore, the data is reused
  - allthe data elements accessed between the source and targetiterations of
    the dependence – the working set – have to beretained in the cache so that
    when the execution reachesthe target iteration, the data element(s) used in
    the sourceiteration will still be present in the cache.
  - We have built a code generator to emit a number of pro-gram variants. The
    code generator creates the loop variantsby applying tiling and loop
    interchange program transforma-tions.
  - The working set sizecomputation analysis is performed on each program
    versiongenerated
  - Among the many variants generated, the rankingalgorithm described below
    picks the top 𝑘 best performingversions, where 𝑘 is a parameter.

- DNN-based code ranking algorithm.
  - assume fully as-sociative, and exclusive caches.
  - If the working set size cor-responding to a data reuse in the program is
    smaller thanthe cache size then the data reuse is exploitable in the cache
  - ranking system considers caches at different levels (typ-ically L1, L2, and
    L3) and for each data reuse, determinesat what level of cache hierarchy is
    the data reuse realizable
  - The algorithm de-termines the fastest level of cache where the working
    setsize corresponding to each data reuse fits and adds it to thatcache’s
    working set size. If a working set does not fit in anycache, then the data
    reuse happens out of the main memory
  - We use a deep neural network (DNNs) for ranking of codevariants.
  - We train the DNN model to perform rela-tive ordering of two code variants.
  - tournamentbased ranking system to assign ranks to the different
    codeversions created
  - we play each code variant against everyother code variant. For each
    variant, we record the numberof wins it has accumulated. We then rank the
    variants basedon the number of wins – the higher the number of wins,
    thehigher the rank
  - We use a four layer feed forward neural network architec-ture
  - e normalize the compiler generatedworking set sizes using min-max scaling
  - The output layerconsists of two neurons and we use the softmax functionfor
    the output layer
  - If theoutput value is above a threshold - 𝜃 , we consider it a 1,
    oth-erwise a 0
  - In this work, we setthe threshold 𝜃 to 0.7
  - depth beyond four did not have anydiscernible effect on accuracy.

## LOW-LEVEL TARGET SPECIFIC INNERLOOP OPTIMIZATIONS
- The low level optimizer focuseson vectorization and assumes that the data
  used by the innerloops is resident in L1 cache.
- To help select the bestvectorization parameters, namely, the unroll factors
  for theloops, we use Reinforcement Learning (RL).

### Vector intrinsic based code generation
- thematrix multiplication code where the j loop is unrolled by afactor of 16
  and the statements are moved to the inner mostloop (unroll-and-jam).
- “we built a code generator that generates vectorized code using vector
  intrinsics” they mean:
  - The compiler is not trusted to auto-vectorize
  - The generator explicitly emits SIMD operations
  - Performance is predictable and portable across CPUs
- using AVX-512intrinsics to run on vector units that can work on 512 bitsof
  data simultaneously. The datatype of the variables in theshown code is 32 bit
  floating point numbers. Therefore, wecan perform arithmetic operations on 16
  floating point num-bers (16 × 32 = 512) at the same time.
- unrolling various loops we can increase the datareuse in vector registers,
  and schedule the load operations insuch a way that memory latency is
  tolerated well
- The interplay betweenthe amount of data reuse, scheduling, and the impact
  ofdata reuse of different arrays could be complex. We use rein-forcement
  learning which in turn uses a neural network todetermine the best unroll
  factors for the loops.

### Reinforcement Learning
- he agentwill suggest whether to increase unroll factors or to decreasethem
- The increment/decrement of the unroll factors formthe actions
- The actions will either lead to a higher performanceor a lower performance
  vis-a-vis the performance of the priorstate
- The agent will use a neural network to suggest next ac-tions to undertake
- we will have two phases – exploration, andexploitation.
  - In the exploration phase, the agent will recom-mend random actions and the
    reward obtained will be usedto continually train the neural network to
    predict actionsthat will lead to larger positive rewards and thus higher
    per-formance states
  - In the exploitation phase, the agent willquery the neural network for the
    best actions – actions thatwill lead to the biggest rewards.
- transitions between the exploration and exploitation phases are controlled
  bythe exploration decay rate.
- The neural network comprises of six intermediate lay-ers – two blocks of
  Dense, Batch Normalization and Dropoutlayers.
  - For Dense layers we use Relu as the activation func-tion. We set the
    drop-out rate of 0.25 for Dropout layers toavoid overfitting
- For matrixmultiplication, there are 7 actions possible: 2 actions for
  eachunroll factor (whether to increment or to decrement) and aspecial state
  to indicate no further action is necessary.

## EXPERIMENTAL EVALUATION

### Evaluation of Low-level optimizations
- As the problem size increases, thework to be performed increases, and due to
  Instruction LevelParallelism (ILP) and because the SIMD units can be keptmore
  busy, the performance goes up

### Evaluation of High-level and Low-leveloptimizations together
[[ PENDING ]]

# DSP-MLIR: A MLIR Dialect for Digital SignalProcessing

## Introduction
- For implementation of software DSPblocks, there is a compiler required to
  convert this algorithmwritten in high-level language into low-level
  programminglanguage(i.e. object code or machine code)
- MLIR (Multi-Level Intermediate Representation), anemerging compiler
  infrastructure under the LLVM umbrella,overcomes this by having multiple
  levels of IR from close tothe source language down to machine-level IR
- Many modern applications combine digital signal pro-cessing (DSP) with other
  computational tasks, such as deeplearning ie, two different domains
  - example, speech recog-nition systems like Google Assistant, Siri, and
    Amazon Echorequire audio preprocessing and deep learning
- The availability of a dedicated DSP dialectin MLIR would enable seamless
  integration and unlock un-precedented cross-domain optimizations, which
  traditionalcompilers cannot achieve
- The DSP language is first compiled into the DSPdialect, and then lowered to
  Affine (whenever possible) orSCF, and then to LLVM IR and Clang

## Background and Related Work
- Dialects act as namespace for defining operations (to definefunctionality),
  types (to define datatype) and attributes (toget compile time information
  ex-constant data about opera-tion)
- There has been previous work in designing DSL for DSP
  - Fieldspar [2] (which provides dataflow style of algorithmdescription based
    on Haskell and the backend compiler pro-duces C code and the optimizations
    (variable elimination,loop unrolling) are done at C-level)
  -  FAUST [14] , anotherfunctional programming based language developed for
     audioprocessing for the Web
- There has been work targetting **backend DSP hardware andC/assembly-level
  software like DSP Processors co-designwith compiler [23] and for generating
  optimized code for DSPDSP-MLIRProcessors using SIMD [17] , Parallelization of
  C programs[6], assembly-level optimizer [4], DSP processors
  addressoptimization [15], enabling auto vectorized code generation[21], etc
  while our work targets the frontend for DSP**

## Proposed DSP MLIR Framework

### The DSP Dialect
- DSP Dialect provides two set of operations
  - one for DSPspecific blocks (like signal operations (like delay),
    transforms(like dft , dct , idft), filter operations (like low-pass filter
    de-sign, response))
  - other for auxiliary operations (like sin,cos, vector of given size
    generation and print)
-  MLIR already supports tensor typeswhich is sufficient to represent DSP
   datatypes which aremostly one-dimensional

### Domain Specific Patterns -Dialect Optimizations
-  There isalso a specific kind of transformation aimed at simplifying
   operations (operation being the main unit of abstractionand transformation)
   called Operation Canonicalization in MLIR, which derives from the base pass
   manager
- Patterns (Optimizations) in DSP dialect can be defined asreplacing a group of
  computationally expensive dsp opera-tions with cheaper operations

#### Optimization 1 - Ideal filter and Cosine-Window Multiplication
- As both ofthem are symmetric about mid-point, the multiplication will also be
  symmetric and hence we have the opportunity to reduce the number of
  calculations to half by just calculating the first as shown in the below
  equation (4).
- Rest half is justsame as first half i.e., ℎ[𝑛] = ℎ[𝐿 − 1 − 𝑛].

#### Optimization 2 - FilterResponse at Noisy signaland Symmetric filter
- By using (8), we reduce the number of load instructions for the filter and we
  get performance benefits

#### Optimization 3 - FilterResponse/Conv1D Prop-erty at input and reverse
input
- According to filter re-sponse property, we know that when the inputs are
  vectorand reverse vector ie, ℎ[𝑙] = 𝑥 [−𝑙] 𝑜𝑟, ℎ[𝑙] = 𝑥 [𝐿 − 1 −𝑙] then the
  output of filter response will be symmetric about its mid-point ie, 𝐿+1 
- So we check the pattern for the operands of filter response and if the inputs
  are reverse of each other, we calculate the output for first half only as
  shown in the below equation (10) and for the second half, we use 𝑦 [𝑛] = 𝑦 [𝑁
  − 1 − 𝑛]

####  Optimization 4 - DFT Response at SymmetricInput
- DFT: Discrete Fourier Transform

#### Optimization 5 - Parsevaal’s Theorem

#### Optimization 6 - Loop fusion for DFTReal andDFTImg Part
- we calculate DFTReal and DFTImgpart separately 
- When the inputs to both the operations are same, affine loop fu-sion should
  fuse the operations into one but we observe itis unable to do so

####  Optimization 7 - LMSFilter and Gain
- LMS Filteris an adaptive filter that aims to minimize the mean squareerror
  between the desired signal 𝑑 (𝑛) and the output signal𝑦 (𝑛)
  -  filter is widely used for applications such as hear-ing aid, noise
     cancelling and echo cancelling
- LMS andGain blocks are used together for hearing aid applicationwhere the
  gain can be fused with LMSFilter

## The DSP Dialect Lowering
- multiple stages to makeour development smoother
  - stages are C-level, Affine-level IR and affine C++ code
-  For each stage, we develop thecode and compare the output with standard
   libraries likenumpy/Matlab
- The second stage is Affine-level IR and it is C-likebut with slight
  differences and restrictions so if our C-codecan’t be expressed as affine
  loop iterations , we again moveto stage first and try another C-code
  resolving those restric-tions like SSA restrictions, constant index etc.
  - Affine-level IR can be tested with mlir-opt whichis available from MLIR
    framework and validated against de-sired output

### The DSP Domain-Specific Language
- Lexer produces the token/symbols which are thenused by recursive Parser to
  generate AST
- Parser parses the module (source file) , which is madeof functions and
  function is made of statements and pro-duces moduleAST
- there is another Parser (mlirGenclass) which takes the moduleAST as input and
  generatescorresponding operation based on statement types from DSLAST which
  will generate the final MLIR IR as an output

## Evaluation Setup
- For measuring the execution time of ouroptimizations, we also used the
  following methods to obtainour time
  - Took average of 5 iterations to take out systemeffect
  - Flushed out cache after any run so that we seesame effect for every run
  - we printed out single elementat random index

## Results
- utilizing the dsp domain spe-cific properties/theorems are much easier at dsp
  dialect levelwhich yields much better performing code
- becausewe are able to exploit the dsp theorem level optimizations atour
  dialect level which is not possible at any other lowering
- canonical optimizations yields around 2xperformance improvement as compared
  to Affine optimiza-tion.
  - For example, for implementing the symmetric filterproperty at C-level in
    Table 2 ( Opt 1 ), this would require,complex polyhedral (mathematical
    abstractions) analysiswhich would lead to complex code implementations
    whilewe do this simply by utilizing the domain knowledge aboutthe filters

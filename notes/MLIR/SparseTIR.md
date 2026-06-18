Rule-Based Automatic Format Selection (Subset of Auto-Format)What it means: The
paper notes that format decomposition rules are currently written manually.
While a fully generic machine-learning-based auto-formatter is a massive
undertaking, a heuristics/rule-based format selector is perfect for 2 months.
Why it's doable: You can write a python-based profiling script or analyzer that
inspects a sparse matrix at compile-time (e.g., looking at row-length variance
to detect power-law distributions) and automatically maps it to a predefined
template like hyb(c, k).  Suggested 2-Month Scope: Write a static analysis pass
that looks at a graph matrix's density pattern and outputs whether it should
use CSR, BSR, or the paper's custom hybrid format.

[SparseTIR](https://arxiv.org/pdf/2207.04606)

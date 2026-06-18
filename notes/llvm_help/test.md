This PR implements optimization for the `ISD::CTLZ` and `ISD::CTLZ_ZERO_UNDEF` for X86 SSE2 target. 

It builds upon #167034 

Closes #161746
## Performance Analysis

<details>
<summary>llvm-mca analysis</summary>

The optimization is benchmarked on `testv4i32` from the `vector-lzcnt-128.ll` file with `-mcpu=core2`.

</details>


| Metric | Before Optimization | After Optimization |
| :--- | :--- | :--- |
| **Iterations** | 100 | 100 |
| **Instructions** | 3900 | 2900 |
| **Total Cycles** | 4569 | 1507 |
| **Total uOps** | 4100 | 3400 |
| **IPC** | 0.85 | 1.92 |
| **uOps Per Cycle** | 0.90 | 2.26 |
| **Block RThroughput** | 10.3 | 8.5 |



| Metric | Before Optimization | After Optimization |
| :--- | :--- | :--- |
| **Iterations** | 100 | 100 |
| **Instructions** | 3900 | 2900 |
| **Total Cycles** | 4569 | 1507 |
| **Total uOps** | 4100 | 3400 |
| **IPC** | 0.85 | 1.92 |
| **uOps Per Cycle** | 0.90 | 2.26 |
| **Block RThroughput** | 10.3 | 8.5 |



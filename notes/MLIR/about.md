> Please note that we now prefer the term 'sparsifier' over the also commonly
> used 'sparse compiler' terminology to refer to such a pass to make it clear
> that the sparsifier pass is not a separate compiler, but should be an
> integral part of any compiler pipeline that is built with the MLIR compiler
> infrastructure
[^src](https://developers.google.com/mlir-sparsifier/guides/intro#:~:text=Please,infrastructure)

# MLIR:
- `Dialect`: related operators `Ops` grouped together in a shared `namespace`
- `Operations (Ops)`: `instructions` of the dialect. ex: a high level ML
  dialect would have `tosa.matmul`

## Dialect Lowering A compilation pass is required (developed with hand) in
order to lower one dialect to the other, using MLIR's `Dialect Conversion
Framework`
- `Conversion Target`: To convert dialect A to dialect B: pass writer specifies
  dialect B as `legal` and A as `illegal`
- `Rewrite Patterns`: Conversion happens using a list of conversion patterns.
- Partial Conversion: match pattern, if nothing found, its fine don't error
- Full Conversion: match pattern, if nothing found, throw error (ie there still
  exists illegal dialect)
- Type conversion is also handled

## Writing an MLIR Pass
- MLIR is extensible
- Writing a pass easier than LLVM
- Uses `TableGen` (declarative codegen tool), can also use C++, and `Pass
  Manager`

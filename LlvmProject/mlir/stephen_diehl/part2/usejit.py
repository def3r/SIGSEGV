import os
import subprocess
import numpy as np
from ctypes import CFUNCTYPE, POINTER
from np_memref import MemRefDescriptor, numpy_to_memref

import llvmlite

llvmlite.opaque_pointers_enabled = True

import llvmlite.binding as llvm

# llvm.initialize() # deprecated
llvm.initialize_native_target()
llvm.initialize_native_asmprinter()

def compile_mlir_to_llvm(mlir_file_path):
    opt_cmd = [
        "mlir-opt",
        mlir_file_path,
        "--convert-tensor-to-linalg",
        "--convert-linalg-to-loops",
        "--convert-scf-to-cf",
        "--convert-cf-to-llvm",
        "--convert-math-to-llvm",
        "--convert-arith-to-llvm",
        "--convert-func-to-llvm",
        "--convert-index-to-llvm",
        "--finalize-memref-to-llvm",
        "--reconcile-unrealized-casts",
    ]

    try:
        opt_result = subprocess.run(opt_cmd, capture_output=True, text=True, check=True)
    except subprocess.CalledProcessError as e:
        print("Err running mlir-opt:")
        print("STDOUT: ", e.stdout)
        print("STDERR: ", e.stderr)
        raise

    translate_cmd = ["mlir-translate", "--mlir-to-llvmir"]
    try:
        translate_result = subprocess.run(
            translate_cmd,
            input=opt_result.stdout,
            capture_output=True,
            text=True,
            check=True,
        )
    except subprocess.CalledProcessError as e:
        print("Error running mlir-translate:")
        print("STDOUT:", e.stdout)
        print("STDERR:", e.stderr)
        raise

    return translate_result.stdout

def create_execution_engine():
    target = llvm.Target.from_default_triple()
    target_machine = target.create_target_machine()
    backing_mod = llvm.parse_assembly("")
    engine = llvm.create_mcjit_compiler(backing_mod, target_machine)
    return engine

def compile_and_load_mlir(mlir_file_path):
    llvm_ir = compile_mlir_to_llvm(mlir_file_path)

    mod = llvm.parse_assembly(llvm_ir)
    mod.verify()

    engine = create_execution_engine()
    engine.add_module(mod)
    engine.finalize_object()

    return engine, mod


def main():
    current_dir = os.path.dirname(os.path.abspath(__file__))
    mlir_file = os.path.join(current_dir, "array_add.mlir")

    engine, mod = compile_and_load_mlir(mlir_file)

    func_ptr = engine.get_function_address("_mlir_ciface_array_add")

    # ctype func wrapper
    array_add = CFUNCTYPE(
        None,
        POINTER(MemRefDescriptor),
        POINTER(MemRefDescriptor),
        POINTER(MemRefDescriptor),
    )(func_ptr)

    size = 1024
    a = np.ones(size, dtype=np.float32)
    b = np.ones(size, dtype=np.float32) * 2
    c = np.zeros(size, dtype=np.float32)

    a_desc = numpy_to_memref(a)
    b_desc = numpy_to_memref(b)
    c_desc = numpy_to_memref(c)

    array_add(a_desc, b_desc, c_desc)

    # verify
    expected = a + b
    np.testing.assert_array_almost_equal(c, expected)
    print("Array addition successful!")
    print(f"first few elements: {c[:5]}")


if __name__ == "__main__":
    main()

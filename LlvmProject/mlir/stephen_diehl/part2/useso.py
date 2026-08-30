import ctypes
import numpy as np
from np_memref import MemRefDescriptor, numpy_to_memref


def main():
    lib = ctypes.CDLL("./libarray_add.dylib")

    # load c interface func
    array_add = lib._mlir_ciface_array_add
    array_add.argtypes = [
        ctypes.POINTER(MemRefDescriptor)
    ] * 3 # inp1, inp2, output
    array_add.restype = None

    size = 1024
    a = np.ones(size, dtype=np.float32)
    b = np.ones(size, dtype=np.float32) * 2
    c = np.zeros(size, dtype=np.float32)

    a_desc = numpy_to_memref(a)
    b_desc = numpy_to_memref(b)
    c_desc = numpy_to_memref(c)

    array_add(ctypes.byref(a_desc), ctypes.byref(b_desc),
              ctypes.byref(c_desc))

    expected = a + b
    np.testing.assert_array_almost_equal(c, expected)
    print("Array addition successful!")
    print(f"first few elements: {c[:5]}")


if __name__ == "__main__":
    main()

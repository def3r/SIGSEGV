import ctypes
import numpy as np

a = np.array([[1, 2, 3], [4, 5, 6]], dtype=np.float32)

print(a.shape)
print(a.strides)
print(a.dtype)
print(a.itemsize)
print(a.ctypes.data)


print (a.ctypes.data_as(ctype.POINTER(ctypes.c_float)))

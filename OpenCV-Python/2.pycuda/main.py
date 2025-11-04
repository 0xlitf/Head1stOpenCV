import pycuda.autoinit
import pycuda.driver as drv
import numpy as np
from pycuda.compiler import SourceModule
import os

env = os.getenv('PATH')
print('env: ', env)

os.environ['PATH'] = env + ";" + r"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64"

# 定义CUDA程序
mod = SourceModule("""
    __global__ void add(int *a, int *b, int *c)
    {
        int i = threadIdx.x;
        c[i] = a[i] + b[i];
    }
""")

# 获取CUDA函数
add = mod.get_function("add")

# 定义输入数据
a = np.array([1, 2, 3, 4, 5]).astype(np.int32)
b = np.array([10, 20, 30, 40, 50]).astype(np.int32)

# 定义输出数据
c = np.zeros_like(a)

# 调用CUDA函数
add(drv.In(a), drv.In(b), drv.Out(c), block=(5, 1, 1))

# 输出结果
print(c)

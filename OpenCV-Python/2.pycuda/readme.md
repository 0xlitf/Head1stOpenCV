# CUDA Envs

```
CUDA_PATH=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.4
```

copy these to RapidEE:
```
CUDA_BIN_PATH=%CUDA_PATH%\bin
CUDA_LIB_PATH=%CUDA_PATH%\lib\x64
CUDA_SDK_PATH=C:\ProgramData\NVIDIA Corporation\CUDA Samples\v11.4
CUDA_SDK_BIN_PATH=%CUDA_SDK_PATH%\bin\win64
CUDA_SDK_LIB_PATH=%CUDA_SDK_PATH%\common\lib\x64
```

add cl.exe to PATH
```
C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64
```

add these to PATH
```
%CUDA_BIN_PATH%
%CUDA_LIB_PATH%
%CUDA_SDK_BIN_PATH%
%CUDA_SDK_LIB_PATH%
```



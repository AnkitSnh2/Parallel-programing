.DEFAULT_GOAL := all
CUDA_PATH ?= /usr/local/cuda
HOST_COMPILER ?= g++
NVCC          := $(CUDA_PATH)/bin/nvcc -ccbin $(HOST_COMPILER)

CXXFLAGS=-I$(CUDA_PATH)/samples/common/inc -g -std=c++11
LDFLAGS=-lfreeimage

obj = kernels.o main.o BatchLoad.o utils.o

%.o: %.cpp
	$(NVCC) $(CXXFLAGS) -arch=sm_75 -o $@ -c $<

%.o: %.cu
	$(NVCC) $(CXXFLAGS) -arch=sm_75 -o $@ -c $<

noise: $(obj)
	$(NVCC) $(LDFLAGS) -m64 -gencode arch=compute_75,code=sm_75 -o $@ $^

all: noise

clean:
	rm -f $(obj) noise

.PHONY: all clean

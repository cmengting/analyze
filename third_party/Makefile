include ../scripts/Makefile.lib

KERNEL:=$(shell uname -s)

ifeq ($(KERNEL),Darwin)
NPROC:=$(shell sysctl -n hw.ncpu)
else
NPROC:=$(shell nproc)
endif

all:
	bazel build $(BAZEL_OPTS) @com_google_protobuf//:protoc
	$(MAKE) -C cppcheck -j $(NPROC)
	./build_libgit2

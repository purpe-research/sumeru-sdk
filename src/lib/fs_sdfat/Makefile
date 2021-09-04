include ${SUMERU_DIR}/conf/sumeru-pre.mk

LIB = libfs_sdfat.a

CFLAGS += -I.

CXXFILES += $(wildcard FsLib/*.cpp)
CXXFILES += $(wildcard ExFatLib/*.cpp)
CXXFILES += $(wildcard FatLib/*.cpp)
CXXFILES += $(wildcard common/*.cpp)

include ${SUMERU_DIR}/conf/sumeru-lib-post.mk

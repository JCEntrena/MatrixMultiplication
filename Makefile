CC = g++
CFLAGS = -lrt

all: $(basename $(wildcard *.c *.cc *.cpp))

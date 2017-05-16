all:
	g++ -o seq seq.cpp
	mpic++ -o parcpu parcpu.cpp
	nvcc -o parcuda parcuda.cu -std=c++11

clean:
	$(RM) ./seq ./parcpu ./parcuda

main:
	g++ -std=c++11 -I include -I libs -o ClosedMDCOP-Miner \
		src/algorithm.cpp \
		src/dataset.cpp \
		src/distances.cpp \
		src/object.cpp \
		src/main.cpp

.PHONY: tests
tests:
	g++ -std=c++11 -I include -I libs -o ClosedMDCOP-Miner-tests \
		src/algorithm.cpp \
		src/dataset.cpp \
		src/distances.cpp \
		src/object.cpp \
		tests/main.cpp

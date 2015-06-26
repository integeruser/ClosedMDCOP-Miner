.DEFAULT_GOAL := release
.PHONY: tests

debug:
	g++ -std=c++11 -DDEBUG -I include -I libs -o ClosedMDCOP-Miner-debug \
		src/algorithm.cpp \
		src/dataset.cpp \
		src/distances.cpp \
		src/object.cpp \
		src/main.cpp

release:
	g++ -std=c++11 -I include -I libs -o ClosedMDCOP-Miner -O3 \
		src/algorithm.cpp \
		src/dataset.cpp \
		src/distances.cpp \
		src/object.cpp \
		src/main.cpp

tests:
	g++ -std=c++11 -I include -I libs -o ClosedMDCOP-Miner-tests \
		src/algorithm.cpp \
		src/dataset.cpp \
		src/distances.cpp \
		src/object.cpp \
		tests/main.cpp

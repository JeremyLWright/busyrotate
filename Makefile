CC=gcc
CXX=g++
CXX_FLAGS=-Wall -Werror -O3 -std=gnu++0x

busyrotate: busyrotate.cpp
	$(CXX) $(CXX_FLAGS) busyrotate.cpp -o busyrotate

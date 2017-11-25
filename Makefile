# Author: Daniel Dusek, xdusek21
# Brno, University of Technology
# BMS class of 2017/2018, Project 1

all:
	g++ -O2  -std=c++11 -Wall -Wextra -pedantic -g -o p1 project.cpp

clean:
	rm p1 out.txt
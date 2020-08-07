default: main.o bool_sat.o
	g++ -g main.o bool_sat.o -o sat

main.o: main.cc 
	g++ -g -c main.cc

bool_sat.o: bool_sat.hh bool_sat.cc
	g++ -g -c bool_sat.cc

clean: 
	rm *.o sat
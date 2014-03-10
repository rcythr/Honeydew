
all: library

clean:
	rm -rf obj/
	rm -rf lib/
	rm -rf include/

library: clean
	# Create the obj folder and output directories.
	mkdir obj
	mkdir lib
	mkdir include
	
	# Compile everything into obj
	g++ -std=c++11 -c rfus.cc -o obj/rfus.o
	g++ -std=c++11 -c task.cc -o obj/task.o
	
	#Construct the archive.
	ar rvs lib/librfus.a obj/rfus.o obj/task.o
	
	#Copy necessary include files over to include
	cp rfus.h include/rfus.h
	cp rfus_type.h include/rfus_type.h
	cp task.h include/task.h

tests:
	g++ -std=c++11 -o simple_test examples/simple_test.cc -I. -L. -lrfus

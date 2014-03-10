
all: library tests

clean:
	rm -rf obj/
	rm -rf lib/
	rm -rf include/
	rm -rf bin/

library: clean
	# Create the obj folder and output directories.
	mkdir obj
	mkdir lib
	mkdir include
	
	# Compile everything into obj
	g++ -std=c++11 -Wl,--no-as-needed -pthread -c rfus.cc -o obj/rfus.o
	g++ -std=c++11 -Wl,--no-as-needed -pthread -c task.cc -o obj/task.o
	
	#Construct the archive.
	ar rvs lib/librfus.a obj/rfus.o obj/task.o
	
	#Copy necessary include files over to include
	cp rfus.hpp include/rfus.hpp
	cp rfus_type.hpp include/rfus_type.hpp
	cp task.hpp include/task.hpp
	cp pipeline.hpp include/pipeline.hpp
	cp join_semaphore.hpp include/join_semaphore.hpp

tests:
	mkdir bin
	g++ -std=c++11 -Wl,--no-as-needed -pthread -o bin/round_robin examples/round_robin.cc -Iinclude -Llib -lrfus
	g++ -std=c++11 -Wl,--no-as-needed -pthread -o bin/round_robin_priority examples/round_robin_priority.cc -Iinclude -Llib -lrfus
	g++ -std=c++11 -Wl,--no-as-needed -pthread -o bin/least_busy examples/least_busy.cc -Iinclude -Llib -lrfus
	g++ -std=c++11 -Wl,--no-as-needed -pthread -o bin/least_busy_priority examples/least_busy_priority.cc -Iinclude -Llib -lrfus
	g++ -std=c++11 -Wl,--no-as-needed -pthread -o bin/pipeline_test examples/pipeline_test.cc -Iinclude -Llib -lrfus


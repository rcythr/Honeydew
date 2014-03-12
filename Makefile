
all: library tests

DEBUG = -ggdb

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
	mkdir include/helpers
	
	# Compile everything into obj
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -c rfus.cc -o obj/rfus.o
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -c task.cc -o obj/task.o
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -c helpers/task_wrapper.cc -o obj/task_wrapper.o -I.
	
	#Construct the archive.
	ar rvs lib/librfus.a obj/rfus.o obj/task.o obj/task_wrapper.o
	
	#Copy necessary include files over to include
	cp rfus.hpp include/rfus.hpp
	cp rfus_type.hpp include/rfus_type.hpp
	cp task.hpp include/task.hpp
	cp join_semaphore.hpp include/join_semaphore.hpp
	
	#Copy over necessary helper includes
	cp helpers/pipeline.hpp include/helpers/pipeline.hpp
	cp helpers/task_wrapper.hpp include/helpers/task_wrapper.hpp
	cp helpers/conditional_task.hpp include/helpers/conditional_task.hpp
	cp helpers/outcome_task.hpp include/helpers/outcome_task.hpp
	cp helpers/exception_task.hpp include/helpers/exception_task.hpp

tests:
	mkdir bin
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/round_robin examples/round_robin.cc -Iinclude -Llib -lrfus
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/round_robin_priority examples/round_robin_priority.cc -Iinclude -Llib -lrfus
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/least_busy examples/least_busy.cc -Iinclude -Llib -lrfus
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/least_busy_priority examples/least_busy_priority.cc -Iinclude -Llib -lrfus
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/pipeline_test examples/pipeline_test.cc -Iinclude -Llib -lrfus
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/exception_test examples/exception_test.cc -Iinclude -Llib -lrfus
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/conditional_test examples/conditional_test.cc -Iinclude -Llib -lrfus
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/outcome_test examples/outcome_test.cc -Iinclude -Llib -lrfus



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
	mkdir include/detail
	mkdir bin
	
	# Compile everything into obj
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -c rfus.cc -o obj/rfus.o
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -c task.cc -o obj/task.o
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -c helpers/task_wrapper.cc -o obj/task_wrapper.o -I.
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -c helpers/post_and_wait.cc -o obj/post_and_wait.o -I.
	
	#Construct the archive.
	ar rvs lib/librfus.a obj/rfus.o obj/task.o obj/task_wrapper.o obj/post_and_wait.o
	
	#Copy necessary include files over to include
	cp rfus.hpp include/rfus.hpp
	cp rfus_type.hpp include/rfus_type.hpp
	cp task.hpp include/task.hpp
	cp detail/join_semaphore.hpp include/detail/join_semaphore.hpp
	
	#Copy over necessary helper includes
	cp helpers/pipeline.hpp include/helpers/pipeline.hpp
	cp helpers/task_wrapper.hpp include/helpers/task_wrapper.hpp
	cp helpers/conditional_task.hpp include/helpers/conditional_task.hpp
	cp helpers/outcome_task.hpp include/helpers/outcome_task.hpp
	cp helpers/exception_task.hpp include/helpers/exception_task.hpp
	cp helpers/event_processor.hpp include/helpers/event_processor.hpp

tests: round_robin round_robin_priority least_busy least_busy_priority pipeline_test exception_test conditional_test outcome_test event_test prime_sieve

round_robin:
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/round_robin examples/round_robin.cc -Iinclude -Llib -lrfus

round_robin_priority:
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/round_robin_priority examples/round_robin_priority.cc -Iinclude -Llib -lrfus

least_busy:
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/least_busy examples/least_busy.cc -Iinclude -Llib -lrfus

least_busy_priority:
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/least_busy_priority examples/least_busy_priority.cc -Iinclude -Llib -lrfus

pipeline_test:
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/pipeline_test examples/pipeline_test.cc -Iinclude -Llib -lrfus

exception_test:
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/exception_test examples/exception_test.cc -Iinclude -Llib -lrfus

conditional_test:
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/conditional_test examples/conditional_test.cc -Iinclude -Llib -lrfus

outcome_test:
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/outcome_test examples/outcome_test.cc -Iinclude -Llib -lrfus

event_test:
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/event_test examples/event_test.cc -Iinclude -Llib -lrfus

prime_sieve:
	g++ $(DEBUG) -std=c++11 -Wl,--no-as-needed -pthread -o bin/prime_sieve examples/prime_sieve.cc -Iinclude -Llib -lrfus

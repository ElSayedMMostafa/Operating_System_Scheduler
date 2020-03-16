build:
	gcc priority_queue.h	
	gcc process_generator.c -o process_generator.out
	gcc clk.c -o clk.out
	gcc scheduler.c -o scheduler.out
	gcc process.c -o process.out
	gcc test_generator.c -o test_generator.out
	gcc processA.c -o processA.out
	gcc processB.c -o processB.out
	gcc processC.c -o processC.out
	gcc processD.c -o processD.out

clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./process_generator.out

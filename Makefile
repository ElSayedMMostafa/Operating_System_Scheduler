build:
	gcc priority_queue.h	
	gcc process_generator.c -o pg.out
	gcc test_generator.c -o test_generator.out
	gcc clk.c -o clk.out
	gcc process.c -o process.out
	gcc hpf.c -o hpf.out
	gcc SRTN.c -o srtn.out
	gcc RR.c -o rr.out


clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./process_generator.out

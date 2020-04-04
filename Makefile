build:
	gcc priority_queue.h	
	gcc process_generator.c -o pg.out
	gcc test_generator.c -o test_generator.out
	gcc clk.c -o clk.out
	gcc process.c -o process.out
	gcc hpf_scheduler.c -o hpf.out -lm
	gcc SRTN_scheduler.c -o srtn.out -lm
	gcc rr_scheduler.c -o rr.out -lm


clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./process_generator.out

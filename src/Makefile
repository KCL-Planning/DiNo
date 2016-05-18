all: dino pddl2upm

dino:
	cd DiNo && make
	
pddl2upm:
	cd DiNo_parser && make
	
clean:
	cd DiNo && make clean
	cd DiNo_parser && make clean

cleanall: 
	cd DiNo && make cleanall
	rm -f ../bin/dino
	cd DiNo_parser && make cleanall
	rm -f ../bin/pddl2upm



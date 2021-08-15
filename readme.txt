1.  Use the following command in a linux machine to compile:
	g++ VC-cs20mtech14008.cpp -o VC-cs20mtech14008.o -lpthread
	g++ SK-cs20mtech14008.cpp -o SK-cs20mtech14008.o -lpthread

2.  Run the compiled code using the following code:
	./VC-cs20mtech14008.o
	./SK-cs20mtech14008.o

3.  Log for VC and SK is generated in outputfile1.txt and outputfile2.txt respectively.

4.  On reruning the code, it may give socket binding error. This can be resolved by changing PORT_NO at line 19.

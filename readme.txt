1.  Use the following command in a linux machine to compile:
	g++ VC.cpp -o VC.o -lpthread
	g++ SK.cpp -o SK.o -lpthread

2.  Run the compiled code using the following code:
	./VC.o
	./SK.o

3.  Log for VC and SK is generated in outputfile1.txt and outputfile2.txt respectively.

4.  On reruning the code, it may give socket binding error. This can be resolved by changing PORT_NO at line 19.

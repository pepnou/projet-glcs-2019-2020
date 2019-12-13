all: heat.out mean.out
	

heat.out: heat.o hdf5IO.o
	h5pcc -Wall -Werror `pkg-config --cflags --libs glib-2.0` heat.o hdf5IO.o -o heat.out
	

heat.o: heat.c hdf5IO.h
	h5pcc -Wall -Werror -c heat.c -o heat.o


hdf5IO.o: hdf5IO.c
	h5pcc -Wall -Werror `pkg-config --cflags --libs glib-2.0` -c hdf5IO.c -o hdf5IO.o 
	

mean.out: mean.o hdf5IO.o
	h5pcc -Wall -Werror mean.o hdf5IO.o `pkg-config --cflags --libs glib-2.0` -o mean.out


mean.o: mean.c hdf5IO.h
	h5pcc -Wall -Werror -c mean.c -o mean.o


runHeat: heat.out
	mpirun -np 4 ./heat.out 100 100 100
	

runMean: mean.out runHeat
	./mean.out 1 2

clean: cleanH5
	rm -f *.o
	rm -f *.out
	

cleanH5:
	rm -f *.h5

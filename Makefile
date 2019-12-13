CC=h5pcc
CFLAGS=-O0 -Wall -Werror `pkg-config --cflags --libs glib-2.0`
LFLAGS=



all: heat.out mean.out derivative.out
	

%.o: %.c hdf5IO.h
	$(CC) $(CFLAGS) -c $< -o $@
	

%.out: %.o hdf5IO.o
	$(CC) $(CFLAGS) $^ -o $@
	

runHeat: heat.out
	mpirun -np 4 ./$< 4 4 8
	

runMean: mean.out runHeat
	mpirun -np 4 ./$< 1 2
	

runDerivative: derivative.out runHeat
	mpirun -np 4 ./$< 2 4
	

clean: cleanH5
	rm -f *.o
	rm -f *.out
	

cleanH5:
	rm -f *.h5

tar: clean
	tar -czf ../projet-GLCS-PEPIN-EMERY.tar.gz ./



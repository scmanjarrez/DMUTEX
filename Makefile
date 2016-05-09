include MakeVars

DIRS=Proceso Controlador

ifeq ($(OS),SunOS)
	LIB:=$(LIB) -lsocket -lnsl -lrt
endif

CONTROLADOR_OBJ=$(shell cat Controlador/.INDEX)
PROCESO_OBJ=$(shell cat Proceso/.INDEX)

all : dirs bin

dirs :
	@(for i in $(DIRS) ; \
          do $(MAKE) -C $$i ; \
          done)

bin : proceso controlador

controlador : $(CONTROLADOR_OBJ)
	$(CC) $(CFLAGS) -o controlador $(CONTROLADOR_OBJ) $(LIB)

proceso : $(PROCESO_OBJ)
	$(CC) $(CFLAGS) -o proceso $(PROCESO_OBJ) $(LIB)

clean : clean-dirs
	rm -f *.o
	rm -f *~
	rm -f controlador proceso core

clean-dirs :
	@(for i in $(DIRS) ; \
          do $(MAKE) -C $$i clean ; \
          done)

entrega : clean
	@(tar cf ../dmutex.2015.entrega.tar Proceso ; \
	rm -f ../dmutex.2015.entrega.tar.gz ; \
	gzip -9 ../dmutex.2015.entrega.tar  ; \
	mv ../dmutex.2015.entrega.tar.gz .  ; \
	echo "Fichero de entrega generado dmutex.2015.entrega.tar.gz")
	  

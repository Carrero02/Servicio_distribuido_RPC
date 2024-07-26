CC = gcc
CLAVES_PATH = claves
FUNCIONES_SERVIDOR_PATH = funciones_servidor
CFLAGS = -lrt -lpthread
OBJS = servidor cliente_tests cliente_concurrente
BIN_FILES = servidor cliente_tests cliente_concurrente

all: $(OBJS)

libclaves.so: $(CLAVES_PATH)/claves.c
	$(CC) -fPIC -c -o $(CLAVES_PATH)/claves.o $<
	$(CC) -shared -fPIC -o $@ $(CLAVES_PATH)/claves.o

libserverclaves.so: $(FUNCIONES_SERVIDOR_PATH)/funciones_servidor.c
	$(CC) -fPIC -c -o $(FUNCIONES_SERVIDOR_PATH)/funciones_servidor.o $<
	$(CC) -shared -fPIC -o $@ $(FUNCIONES_SERVIDOR_PATH)/funciones_servidor.o

servidor:  servidor.c libserverclaves.so
	$(CC) -L. -lserverclaves -o $@.out $< ./libserverclaves.so $(CFLAGS)

cliente_tests: cliente_tests.c libclaves.so
	$(CC) -L. -lclaves -o $@.out $< ./libclaves.so $(CFLAGS)

cliente_concurrente: cliente_concurrente.c libclaves.so
	$(CC) -L. -lclaves -o $@.out $< ./libclaves.so $(CFLAGS)

clean:
	rm -f $(BIN_FILES) *.out *.o *.so $(CLAVES_PATH)/*.o $(FUNCIONES_SERVIDOR_PATH)/*.o data.txt

re:	clean all

.PHONY: all libclaves.so libserverclaves.so servidor cliente_tests cliente_concurrente clean re
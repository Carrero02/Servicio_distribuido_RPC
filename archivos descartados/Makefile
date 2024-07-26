CC = gcc
CLAVES_PATH = claves
SOCKETS_PATH = sockets
CFLAGS = -lrt -g -I/usr/include/tirpc -fPIC
LDFLAGS = -lnsl -lpthread
OBJS = servidor libclaves.so cliente_tests cliente_concurrente
BIN_FILES = servidor cliente_tests cliente_concurrente
SERVER_OBJS = servidor.o funciones_servidor/funciones_servidor.o claves_svc.o claves_xdr.o
CLIENT_OBJS = claves/claves.o claves_xdr.o claves_clnt.o
CLIENT1 = cliente_tests.o
CLIENT2 = cliente_concurrente.o
LIB_OBJS = claves/claves.o

all: $(OBJS)

libclaves.so: claves/claves.o $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -c -o $<
	$(CC) $(LDFLAGS) -shared -o $@ $(LIB_OBJS) -lrt

servidor: $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(SERVER_OBJS) -o $@.out $(LDFLAGS)

cliente_tests: $(CLIENT1) libclaves.so
	$(CC) $(CFLAGS) $(CLIENT1) $(CLIENT_OBJS) -o $@.out $(LDFLAGS) -L. -lclaves

cliente_concurrente: $(CLIENT2) libclaves.so
	$(CC) $(CFLAGS) $(CLIENT2) $(CLIENT_OBJS) -o $@.out $(LDFLAGS) -L. -lclaves

clean:
	rm -f $(BIN_FILES) *.out *.o *.so $(CLAVES_PATH)/*.o tuplas.txt

re:	clean all

.PHONY: all libclaves.so servidor cliente_tests cliente_concurrente clean re
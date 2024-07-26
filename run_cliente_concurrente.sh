#!/bin/bash
# Add the current directory to the library path so that the linker can find the shared library
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
export IP_TUPLAS=localhost
./cliente_concurrente.out
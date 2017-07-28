all: compilar

compilar:
	gcc -shared -fPIC -o "libfunciones.so" ../cliente-servidor.c ../cliente-servidor.h ../deserializador.c ../deserializador.h ../estructurasCompartidas.c ../estructurasCompartidas.h ../logger.c ../logger.h ../serializador.c ../serializador.h

install: compilar
	cp -u libfunciones.so /usr/lib
	cp -u ../cliente-servidor.h /usr/include
	cp -u ../deserializador.h /usr/include
	cp -u ../estructurasCompartidas.h /usr/include
	cp -u ../logger.h /usr/include
	cp -u ../serializador.h /usr/include
	make clean

uninstall:
	rm -f /usr/lib/funciones.so
	rm -rf /usr/include/cliente-servidor.h 
	rm -rf /usr/include/deserializador.h
	rm -rf /usr/include/estructurasCompartidas.h 
	rm -rf /usr/include/logger.h 
	rm -rf /usr/include/serializador.h 

clean:
	rm -f funciones.so

.PHONY: all install uninstall compilar

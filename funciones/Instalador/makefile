all: compilar

compilar:
	gcc -shared -fPIC -o "funciones.so" ../funciones/cliente-servidor.c ../funciones/cliente-servidor.h ../funciones/deserializador.c ../funciones/deserializador.h ../funciones/estructurasCompartidas.c ../funciones/estructurasCompartidas.h ../funciones/logger.c ../funciones/logger.h ../funciones/serializador.c ../funciones/serializador.h

install: compilar
	cp -u funciones.so /usr/lib
	cp -u ../funciones/cliente-servidor.h /usr/include
	cp -u ../funciones/deserializador.h /usr/include
	cp -u ../funciones/estructurasCompartidas.h /usr/include
	cp -u ../funciones/logger.h /usr/include
	cp -u ../funciones/serializador.h /usr/include
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

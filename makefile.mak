
all: 
	-cd kernel && $(MAKE) all
	-cd memoria && $(MAKE) all
	-cd cpu && $(MAKE) all
	-cd fylesystem && $(MAKE) all
	-cd consola && $(MAKE) all

clean:
	-cd kernel && $(MAKE) clean
	-cd memoria && $(MAKE) clean
	-cd cpu && $(MAKE) clean
	-cd fylesystem && $(MAKE) clean
	-cd consola && $(MAKE) clean
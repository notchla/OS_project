
default:
	@echo "Please specify a target"

umps:
	$(MAKE) -f umpsmake
	
umps2:
	$(MAKE) -f umpsmake

uarm:
	$(MAKE) -f uarmmake

clean:
	$(MAKE) -f uarmmake clean
	$(MAKE) -f umpsmake clean

.PHONY: umps umps2 uarm clean default
all:
	@$(foreach var,$(SUBDIRS),cd $(var) && make && cd .. ;)

install:
	@$(foreach var,$(SUBDIRS),cd $(var) && make install && cd .. ;)

clean:
	@$(foreach var,$(SUBDIRS),cd $(var) && make clean && cd .. ;)


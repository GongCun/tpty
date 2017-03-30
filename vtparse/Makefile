INSTALL_PATH = /usr/local/bin

RUBY_GENERATION_FILES = vtparse_gen_c_tables.rb vtparse_tables.rb

all: vtparse_table.c vtparse_table.h vtparse_test vtclean

clean:
	rm -rf vtparse_table.c vtparse_table.h vtparse_test vtclean *.dSYM

vtparse_table.c: $(RUBY_GENERATION_FILES)
	ruby vtparse_gen_c_tables.rb

vtparse_table.h: $(RUBY_GENERATION_FILES)
	ruby vtparse_gen_c_tables.rb

vtparse_test: vtparse.c vtparse.h vtparse_table.c vtparse_table.h vtparse_test.c
	gcc -Wall -g -o $@ vtparse_test.c vtparse.c vtparse_table.c

vtclean: vtparse.c vtparse.h vtparse_table.c vtparse_table.h vtclean.c
	gcc -Wall -g -o $@ vtclean.c vtparse.c vtparse_table.c
	@[ -d ../bin/ ] || mkdir -p ../bin
	@cp -p $@ ../bin

install: vtclean
	@[ -d $(INSTALL_PATH)/ ] || \
		(mkdir -p $(INSTALL_PATH)/; chmod 755 $(INSTALL_PATH)/)
	install -s -S -f $(INSTALL_PATH)/ -M 755 -O root -G system $@

.PHONY: all clean


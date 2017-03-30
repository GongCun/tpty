TEMPFILES=core core.* temp.* *.out *~
MAN_PATH=/usr/local/share/man

PROGS = tpty.1.gz tptyreplay.1.gz tpty.man tptyreplay.man

all: $(PROGS)

tpty.1.gz: tpty.1
	@tar -cvf - $< | gzip -c >$@

tpty.man: tpty.1
	@nroff -man $< | col -b >$@
	@cp -p $@ ../

tptyreplay.1.gz: tptyreplay.1
	@tar -cvf - $< | gzip -c >$@

tptyreplay.man: tptyreplay.1
	@nroff -man $< | col -b >$@
	@cp -p $@ ../

install: tpty.1 tptyreplay.1
	@[ -d $(MAN_PATH)/man1/ ] || \
		(mkdir -p $(MAN_PATH)/man1/; chmod 755 $(MAN_PATH)/man1/)
	@cp -p $^ $(MAN_PATH)/man1

clean:
	rm -rf $(PROGS) $(TEMPFILES)

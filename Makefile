all: list_micro

list_micro: list_micro.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@-rm -rf list_micro > /dev/null 2>&1

.PHONY: clean all

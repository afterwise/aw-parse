
test: test.o ../libaw-parse.a
	clang -o $@ $^

%.o: %.c
	clang -I.. -c $< -o $@

../libaw-parse.a:
	$(MAKE) -C..

.PHONY: clean
clean:
	rm -f test test.o


all: clean cds shell run_cds

clean:
	rm -rf build/
	mkdir build/
	rm -rf astreal/
	mkdir astreal/

cds:
	clang src/cds.c -o build/cds

shell:
	clang src/shell.c -o build/shell

run_cds:
	build/cds

run_shell:
	sleep 0.2
	build/shell

count:
	@find . -name '*.h' -o -name '*.c' | xargs wc -lc
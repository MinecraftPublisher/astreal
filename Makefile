all: clean cds shell run_cds

clean:
	rm -rf build/
	mkdir build/

cds:
	clang src/cds.c -o build/cds

shell:
	clang src/shell.c -o build/shell

run_cds:
	build/cds

run_shell:
	build/shell
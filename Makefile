all: clean cds shell stages count

clean:
	rm -rf build/
	mkdir build/
	rm -rf astreal/
	mkdir astreal/

cds:
	clang -rdynamic src/cds.c -o build/cds

shell:
	clang -rdynamic src/shell.c -o build/shell

run_cds:
	build/cds

run_shell:
	sleep 0.2
	build/shell

stages:
	clang -shared -fPIC src/stages/stages.c -o build/stages.so

count:
	@echo
	@echo "Line count:"
	@find . -name '*.h' -o -name '*.c' | xargs wc -lc
all: parser main

parser: # Install peggy.js for this to work.
	peggy parser.pegjs  --allowed-start-rules \* --format es

helper:
	clang helper.c -o yippee_helper

main:
	deno run  --allow-all main.ts --log-file

file_out:
	deno run  --allow-all main.ts --log-file > output.json
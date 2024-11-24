all: parser main

parser: # Install peggy.js for this to work.
	peggy parser.pegjs  --allowed-start-rules \* --format es

main:
	deno run  --allow-env --allow-read --allow-all main.ts
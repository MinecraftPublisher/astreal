# astreal - An ambitious journey through the high seas

Forgive me if this README is written like a journal entry, but I felt like this was the right thing to do.

I wanted to create a game. It would have a text interface, and it would involve extremely complex space-bending movement, crammed into the text setting.

I decided to use C for it.

I created my own tools to handle dynamically sized arrays, but encountered a critical bug in the process, halting the development process. By that time, I was plenty frustrated by the C language, so I decided to make my own.

I created astreal, the programming language, and it was meant to be a C-like language with low overhead (meaning high runtime speeds), whilst also providing rust's memory safety without its borrow checker shenanigans.

The compiler was meant to feature easy-to-use syntax, a better alternative to object-oriented programming that still encourages meaningful and readable code instead of whatever the hell can be found in corporation C#/ASP.NET codebases (you'll know what I mean if you've looked at one before).

The compiler can identify unsued code, and easily eliminate it. One of the features that I'm most proud of in this project is a purity checker, which can tell which parts of the code are pure (AKA they cause no side effects) and which parts are not, and this allowed for very potent code elimination.

But I was too ambitious. Like I always am. I got stuck, I couldn't finish the project in time for the deadline (speaking as it's currently Jan 27th), and I procrastinated.

In that time frame, I created [alan](https://github.com/MinecraftPubliher/alan), a project which I'm very proud of. It takes code and compiles it directly to machine code, no dependencies used inbetween. No assembler, no LLVM, no Clang, no nothing. The compiler is a singular standalone program with no dependencies.

But then, I spent a bit too much time on alan (specifically 87 hours), whilst I had spent just 44 hours writing Astreal.

Anyways, the demo video showcases how the compiler can generate an abstract syntax tree, and how it handles input code.

Thank you for reading this through! Have a nice day/night.
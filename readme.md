# idk - implementing deep keyboard

Transpiling a imaginary language to Nasm for Linux-x86_64.

Currently, using Kotlin as the interpreter and compiler, but will change in the future.

**WARN:** Don't try to learn from this. Not worth.

## Current plans required for turing complete:

### Turing completion needs:
- Basic math: +, -, *, /
- Conditions: if
- Loop: while
- Memory Access: read and right

### Function declaration and call:
- Functions require blocks even if they are empty.
- The keyword is `fn`
- Calling a function require semicolons at the end.
```rust
fn main() {
    println("Hello world");
}
```
### Variable declaration, assign and access:
- Mutable or not, they can be represented with `var` for mutable and `val` for immutable.
- Require a semicolon at the end. Like the mom of all languages.
```rust
var a = 69; //Mutable
val b = 79; //Const
a = 420;
b;
```
### Control Flow: (Experimental)
- Parenthesis aren't required, at least for now...
- Pretty much the same thing as all the other languages.
```rust
if x < 0 x = 10;
else y > 10 y = 0;
else x--;
```
### Loops:
- The while loop does everything.
```rust
while x < 1000 doLogic();
```
### Memory access:
- I know that memory access is a bit controversial for security, but this is a hobby language that anyone uses lol.
```rust
mem[addr] = 69;
var b = mem[addr];
```

## How to? If you are interested!

Only working in the imaginary land. So run `./gradlew test` to run the current AST tests.

```
./gradlew test
```
# idk - implementing deep keyboard

Transpiling a imaginary language to Nasm for Linux-x86_64.

Currently, using Kotlin as the interpreter, but will change in the future.

**WARN:** Don't try to learn from this. Not worth.

### How to? If you are interested!

Currently, the only example is a print syscall.
The `gradlew run` generates the `.asm` file and the `./test print` will compile and link the assembly file with the given name.

```
./gradlew run
cd tests
./test print
```
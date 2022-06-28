set -xe

mkdir -p ./build

nasm -f elf64 -o "$1".o "$1".asm
ld "$1".o -o build/"$1"
./build/"$1"
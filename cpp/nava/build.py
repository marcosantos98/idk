import glob;
import subprocess;

IDK="../build/idk"

nava_files=glob.glob("./*.nava")

for src in nava_files:
    subprocess.call([IDK, src])

asm_files=glob.glob("./*.asm")

for src in asm_files:
    try:
        print("nasm -felf64 %s" % src)
        subprocess.run(["nasm", "-felf64", src], check=True)
    except subprocess.CalledProcessError:
        print("Failed to compile %s" % src)
        exit(1)

o_files=glob.glob("./*.o")

s ="ld -o main "

for src in o_files:
    s += src + ' '

print(s)

subprocess.run(["sh", "-c", s])
subprocess.run(["./main"])

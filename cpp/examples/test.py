#!/usr/bin/python

import glob;
import subprocess;
import sys;
import os;

IDK="../build/idk"

def compile(path: str):
    subprocess.call([IDK, "./" + path])
    subprocess.call([IDK, "./Debug.nava"])
    subprocess.call(["nasm", "-felf64", path.split('.')[0] + ".asm"])
    subprocess.call(["nasm", "-felf64", "Debug.asm"])
    s = "ld -o main " + path.split('.')[0] + ".o Debug.o" 
    subprocess.call(["sh", "-c", s])

def compile_and_run(path: str):
    compile(path)
    s = "./main > " + path.split('.')[0] + ".temp"
    subprocess.call(["sh", "-c", s])
    if open(path.split('.')[0] + ".temp", "r").read() != open(path.split('.')[0] + ".txt", "r").read():
        print("Failed test: %s" % path)
        exit(1)
    else:
        print("Passed test: %s" % path)
    os.remove(path.split('.')[0] + ".temp")

def compile_and_record(path: str):
    compile(path)
    s = "./main > " + path.split('.')[0] + ".txt" 
    subprocess.call(["sh", "-c", s])

def compile_run_all():
    nava_files = glob.glob("./*.nava")
    for nava in nava_files:
        if nava == "./Debug.nava":
            continue
        else:
            compile_and_run(nava.split('/')[1])

def compile_record_all():
    nava_files = glob.glob("./*.nava")
    for nava in nava_files:
        if nava == "./Debug.nava":
            continue
        else:
            compile_and_record(nava.split('/')[1])

if __name__ == "__main__":
    assert len(sys.argv) > 1, "Not enought arguments."
    if(sys.argv[1] == "-a"):
        compile_run_all()
    elif sys.argv[1] == "-r":
        compile_record_all()
    else:
        compile_and_run(sys.argv[1])
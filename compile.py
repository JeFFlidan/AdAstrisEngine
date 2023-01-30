import os
import subprocess


def compile_script():
    root_path = os.getcwd()
    build_path = os.getcwd() + "\\build"
    isDir = os.path.isdir(build_path)
    needsReconfig = True
    if not isDir:
        os.mkdir(build_path)
        subprocess.check_call('cmake -G "MinGW Makefiles" -B ' + build_path, shell=True)
        needsReconfig = False

    if needsReconfig:
        subprocess.check_call('cmake -G "MinGW Makefiles" ' + build_path, shell=True)
    os.chdir(build_path)
    subprocess.check_call('mingw32-make -j4')
    os.chdir(root_path)
    if not os.path.isfile(root_path + "/Makefile"):
        subprocess.check_call('cmake -G "MinGW Makefiles" ' + root_path, shell=True)
    subprocess.check_call('cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ' + os.getcwd(), shell=True)


if __name__ == '__main__':
    compile_script()

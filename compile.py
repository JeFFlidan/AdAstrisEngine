import os
import subprocess


def compile_script():
    build_path = os.getcwd() + "\\build"
    subprocess.check_call('cmake -G "MinGW Makefiles" ' + build_path, shell=True)
    subprocess.check_call('cmake --build ' + build_path, shell=True)
    subprocess.check_call('cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ' + os.getcwd(), shell=True)


if __name__ == '__main__':
    compile_script()

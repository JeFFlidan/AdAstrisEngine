import os
import subprocess
import sys


def get_info():
    print('\nThis script simplifies compilation of engine and all of its plugins.\n')
    print('Usage\n')
    print('\tpython compile.py [options]')
    print("\tIf you don't use any flags, only the engine will be compiled (without plugins).\n")
    print('Options')
    print('\t-engine      = Compile engine source. You have to use it\n'
    '                       if you want to compile the engine and some plugins')
    print('\t-plugins     = Compile plugins. If you use only this flag,\n'
    '                       all plugins will be compiled.')
    print('\t-ecs         = Compile entity-component-system plugin.\n'
    '                       Should be used with the -plugins flag.')
    print('\t-help        = Print information about compile script.')


def compile_commmon(rootPath, buildPath):
    print(buildPath)
    isDir = os.path.isdir(buildPath)
    needsReconfig = True
    if not isDir:
        os.mkdir(buildPath)
        subprocess.check_call('cmake -G "MinGW Makefiles" -B ' + buildPath, shell=True)
        needsReconfig = False

    if needsReconfig:
        subprocess.check_call('cmake -G "MinGW Makefiles" ' + buildPath, shell=True)

    os.chdir(buildPath)
    subprocess.check_call('mingw32-make -j8')
    os.chdir(rootPath)


## TODO remove - before name to avoid error
def compile_plugin(name):
    print("Compile one plugin")
    rootPath = os.getcwd()
    buildPath = rootPath + '\\engine\\plugins\\' + name + '\\build'
    print(rootPath)
    os.chdir(rootPath + '\\engine\\plugins\\' + name)
    compile_commmon(rootPath, buildPath)


def compile_plugins(flags):
    print(f"Flags length 2 : {len(flags)}")
    if (len(flags) == 2) or (len(flags) == 3 and '-engine' in flags):
        print("Compile all plugins")

        pluginsNames = ['ecs']

        for name in pluginsNames:
            compile_plugin(name)

        return

    if '-ecs' in flags:
        compile_plugin('ecs')


def compile_module(name):
    rootPath = os.getcwd()
    buildPath = rootPath + '\\engine\\src\\' + name + '\\build'
    os.chdir(rootPath + '\\engine\\src\\' + name)
    compile_commmon(rootPath, buildPath)


def compile_modules(flags):
    if (len(flags) == 2 or (len(flags) == 3 and '-engine' in flags)):
        moduleNames = ["vulkan_rhi", "render_core"]

        for name in moduleNames:
            compile_module(name)

        return

    if '-vulkan_rhi' in flags:
        compile_module('vulkan_rhi')

    if '-render_core' in flags:
        compile_module('render_core')


def compile_engine():
    rootPath = os.getcwd()
    buildPath = os.getcwd() + "\\build"

    compile_commmon(rootPath, buildPath)

    if not os.path.isfile(rootPath + "/Makefile"):
        subprocess.check_call('cmake -G "MinGW Makefiles" ' + rootPath, shell=True)


def compile():
    flags = sys.argv

    if '-help' in flags:
        get_info()
        return

    if (len(flags) == 1 and '-plugins' not in flags) or '-engine' in flags:
        print("Compile engine")
        compile_engine()


    if '-plugins' in flags:
        print("Compile plugins")
        print(f"Flags length = {len(flags)}")
        compile_plugins(flags)

    if '-modules' in flags:
        compile_modules(flags)


if __name__ == '__main__':
    compile()

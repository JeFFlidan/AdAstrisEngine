import os
import subprocess
import sys


class CmakeBuilder:
    all_compile_flags = {
        '-app' : '-DBUILD_ENGINE_CORE',
        '-third_party' : '-DBUILD_THIRD_PARTY',
        '-project_launcher' : '-DBUILD_PROJECT_LAUNCHER',
        '-vulkan_rhi' : '-DBUILD_VULKAN_RHI',
        '-renderer' : '-DBUILD_RENDERER',
        '-render_core' : '-DBUILD_RENDER_CORE',
        '-engine' : '-DBUILD_LOW_LEVEL_ENGINE',
        '-tests' : '-DBUILD_TESTS'
    }


    fully_dependent_modules = { '-render_core', '-engine', '-renderer', '-app', '-tests' }
    partially_dependent_modules = { '-project_launcher', '-vulkan_rhi' }


    def __init__(self):
        self.flags = sys.argv[1:]
        self.root_path = os.getcwd()
        self.build_path = os.getcwd() + '\\build'
        self.cmake_prepare_command = ''

        is_build_dir = os.path.isdir(self.build_path)
        if not is_build_dir:
            os.mkdir(self.build_path)
        
        if '-help' in self.flags or not self.flags:
            self.__print_info()
            return
        
        subprocess.check_call('cmake --fresh ' + self.build_path, shell=True)
        self.__build_internal()
        
        if '-vs2017' in self.flags:
            subprocess.check_call(f'cmake {self.cmake_prepare_command} -G "Visual Studio 15 2017" -B {self.build_path}', shell=True)
        elif '-vs2019' in self.flags:
            subprocess.check_call(f'cmake {self.cmake_prepare_command} -G "Visual Studio 16 2019" -B {self.build_path}', shell=True)
        elif '-vs2022' in self.flags:
            subprocess.check_call(f'cmake {self.cmake_prepare_command} -G "Visual Studio 17 2022" -B {self.build_path}', shell=True)
        else:
            subprocess.check_call(f'cmake {self.cmake_prepare_command} -G "Visual Studio 17 2022" -B {self.build_path}', shell=True)

        is_bin_dir = os.path.isdir(self.root_path + '\\bin')
        if not is_bin_dir:
            os.mkdir(self.root_path + '\\bin')


    def __build_internal(self):
        if '-all' in self.flags:
            for key in self.all_compile_flags:
                self.__set_cmake_flags(key)
        else:
            is_full_dependency_resolved = False
            is_partial_dependency_resolved = False
            for key in self.flags:
                if key != '-app':
                    self.__set_cmake_flags(key)

                if key in self.fully_dependent_modules and not is_full_dependency_resolved:
                    self.__set_cmake_flags('-app')
                    is_full_dependency_resolved = True
                    if not is_partial_dependency_resolved:
                        self.__set_cmake_flags('-third_party')
                        is_partial_dependency_resolved = True
                elif key in self.partially_dependent_modules and not is_partial_dependency_resolved:
                    self.__set_cmake_flags('-third_party')
        
        
        subprocess.check_call(f'cmake {self.cmake_prepare_command} {self.build_path}', shell=True)


    def __set_cmake_flags(self, cmd_flag):
        self.cmake_prepare_command += self.all_compile_flags[cmd_flag] + '=ON '


    def __print_info(self):
        print('\nThis script simplifies compilation of engine and all of its plugins.\n')
        print('Usage\n')
        print('\tpython compile.py [options]')
        print("\tIf you don't use any flags, help info will be printed.\n")
        print('Options')
        print('\t-help             = Print information about compile script.\n')
        print('\t-all              = Compiles AdAstrisEngine from scratch. Must be used to install the app.\n')
        print('\t-app              = Compiles core application that manages all the modules.\n')
        print('\t-third_party      = Compiles all third party libs.\n')
        print('\t-engine           = Compiles low level Engine module.\n')
        print('\t-vulkan_rhi       = Compiles Vulkan Render Hardware Interface Module.\n')
        print('\t-render_core      = Compiles Render Core Module.\n')
        print('\t-renderer         = Compiles Renderer module.\n')
        print('\t-project_launcher = Compiles Project Launcher Module.\n')
        print('\t-vs2017           = Uses cmake generator for Visual Studio 15 2017')
        print('\t-vs2019           = Uses cmake generator for Visual Studio 16 2019')
        print('\t-vs2022           = Uses cmake generator for Visual Studio 17 2022')


if __name__ == '__main__':
    cmake_builder = CmakeBuilder()

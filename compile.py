import os
import subprocess
import sys


class CompileManager:
    all_compile_flags = {
        '-app' : '-DBUILD_ENGINE_CORE',
        '-third_party' : '-DBUILD_THIRD_PARTY',
        '-project_launcher' : '-DBUILD_PROJECT_LAUNCHER',
        '-vulkan_rhi' : '-DBUILD_VULKAN_RHI',
        '-renderer' : '-DBUILD_RENDERER',
        '-render_core' : '-DBUILD_RENDER_CORE',
        '-engine' : '-DBUILD_LOW_LEVEL_ENGINE'
    }
    
    
    fully_dependent_modules = { '-render_core', '-engine', '-renderer', '-app' }
    partially_dependent_modules = { '-project_launcher', '-vulkan_rhi' }
    
    
    def __init__(self):
        self.flags = sys.argv[1:]
        self.root_path = os.getcwd()
        self.build_path = os.getcwd() + '\\build'
        self.cmake_prepare_command = ''
        self.cmake_cleanup_command = ''
        
        is_build_dir = os.path.isdir(self.build_path)
        if not is_build_dir:
            os.mkdir(self.build_path)
            subprocess.check_call('cmake -G "MinGW Makefiles" -B ' + self.build_path, shell=True)
            
        is_bin_dir = os.path.isdir(self.root_path + '\\bin')
        if not is_bin_dir:
            os.mkdir(self.root_path + '\\bin')
            
        
    def compile(self):
        if '-help' in self.flags or not self.flags:
            self.__print_info()
            return
        
        self.__compile_internal()
                
    
    def __compile_internal(self):
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
        
        self.__compile_mingw()


    def __compile_mingw(self):
        subprocess.check_call(f'cmake {self.cmake_prepare_command} {self.build_path}', shell=True)
        os.chdir(self.build_path)
        subprocess.check_call('mingw32-make -j10')
        os.chdir(self.root_path)
        subprocess.check_call(f'cmake {self.cmake_cleanup_command} {self.build_path}', shell=True)
        
    
    def __set_cmake_flags(self, cmd_flag):
        self.cmake_prepare_command += self.all_compile_flags[cmd_flag] + '=ON '
        self.cmake_cleanup_command += self.all_compile_flags[cmd_flag] + '=OFF '
        
        
    def __print_info(self):
        print('\nThis script simplifies compilation of engine and all of its plugins.\n')
        print('Usage\n')
        print('\tpython compile.py [options]')
        print("\tIf you don't use any flags, help info will be printed.\n")
        print('Options')
        print('\t-help             = Print information about compile script.\n')
        print('\t-all              = Compile AdAstrisEngine from scratch. Must be used to install the app.\n')
        print('\t-app              = Compile core application that manages all the modules.\n')
        print('\t-third_party      = Compile all third party libs.\n')
        print('\t-engine           = Compile low level Engine module.\n')
        print('\t-vulkan_rhi       = Compile Vulkan Render Hardware Interface Module.\n')
        print('\t-render_core      = Compile Render Core Module.\n')
        print('\t-renderer         = Compile Renderer module.\n')
        print('\t-project_launcher = Compile Project Launcher Module.\n')
        
        
if __name__ == '__main__':
    compile_manager = CompileManager()
    compile_manager.compile()

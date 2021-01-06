# uncomment as necessary
project('mlt3send', 'cpp',
        license : ['Boost'],
        version : '0.1',
        default_options : ['cpp_std=c++17', 'warning_level=3'])#, 'werror=true'])

thread_dep = dependency('threads')

inc_dir = include_directories(['include', 
                               get_option('cli11_include_dir'),
                               get_option('spdlog_include_dir'),
                               get_option('tabulate_include_dir'),
                               get_option('rang_include_dir')
                              ])
src = ['src/main.cpp']

executable('mlt3send',
           sources : src,
           include_directories : inc_dir,
           dependencies : [thread_dep]
          )
# uncomment as necessary
project('mlt3send', 'cpp',
        license : ['Boost'],
        version : '0.1',
        default_options : ['cpp_std=c++17', 'warning_level=3', 'werror=true'])

thread_dep = dependency('threads')

# begin asio: https://think-async.com/Asio/
#add_global_arguments('-I' + get_option('asio_include_dir'), language : 'cpp')
#add_global_arguments('-DASIO_STANDALONE', language : 'cpp')
#add_global_arguments('-DASIO_NO_DEPRECATED', language : 'cpp')
# end asio

# CLI11: https://github.com/CLIUtils/CLI11
#add_global_arguments('-I' + get_option('cli11_include_dir'), language : 'cpp')

# spdlog: https://github.com/gabime/spdlog
#add_global_arguments('-I' + get_option('spdlog_include_dir'), language : 'cpp')

# json: https://github.com/nlohmann/json
#add_global_arguments('-I' + get_option('json_include_dir'), language : 'cpp')


inc_dir = include_directories(['include', 
                               get_option('cli11_include_dir')
                              ])
src = ['src/main.cpp']

executable('mlt3send',
           sources : src,
           include_directories : inc_dir,
           dependencies : [thread_dep]
          )
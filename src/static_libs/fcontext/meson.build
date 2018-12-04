
fcontext_src = [
    'make_all_gas.S',
    'jump_all_gas.S'
]

if get_option('coro') == 'fcontext'
    fcontext_lib = static_library('fcontext',
        fcontext_src,
        install: false
    )
endif

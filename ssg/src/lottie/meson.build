ssg_sources_lottie = []

ssg_lottie_file = ['lottiecomposition.cpp',
                   'lottiedrawable.cpp',
                   'jsontest.cpp'
                  ]

foreach file: ssg_lottie_file
   ssg_sources_lottie += join_paths('lottie', file)
endforeach

subdir('animation')
subdir('model')

foreach file: ssg_sources_lottie_model
   ssg_sources_lottie += join_paths('lottie', file)
endforeach

foreach file: ssg_sources_lottie_animation
   ssg_sources_lottie += join_paths('lottie', file)
endforeach

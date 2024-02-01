add_library(minhook
  src/hde/hde32.c
  src/hde/hde64.c
  src/buffer.c
  src/hook.c
  src/trampoline.c)

target_include_directories(minhook
  PRIVATE
    src
  PUBLIC
    include)

#####################################################################
## nspr (Library)
#####################################################################
set(NSPR_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/io/prdir.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/io/prfdcach.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/io/prfile.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/io/prio.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/io/priometh.c "
  "${CMAKE_CURRENT_SOURCE_DIR}/src/io/pripv6.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/io/prlayer.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/io/prlog.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/io/prmapopt.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/io/prmmap.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/io/prmwait.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/io/prpolevt.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/io/prprf.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/io/prscanf.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/io/prsocket.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/io/prstdio.c"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/linking/prlink.c"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/malloc/prmalloc.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/malloc/prmem.c"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/md/windows/ntmisc.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md/windows/ntsec.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md/windows/ntsem.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md/windows/ntinrval.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md/windows/ntgc.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md/windows/w95thred.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md/windows/w95io.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md/windows/w95cv.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md/windows/w95sock.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md/windows/win32_errors.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md/windows/w32ipcsem.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md/windows/w32poll.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md/windows/w32rng.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md/windows/w32shm.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md/windows/w95dllmain.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/md/prosdep.c"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/memory/prseg.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/memory/prshm.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/memory/prshma.c"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/pralarm.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/pratom.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/praton.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prcountr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prdtoa.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prenv.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prerr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prerror.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prerrortable.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prinit.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prinrval.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/pripc.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/pripcsem.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prlog2.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prlong.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prnetdb.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prolock.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prrng.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prsystem.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prthinfo.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prtime.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prtpool.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/misc/prtrace.c"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/threads/combined/prucpu.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/threads/combined/prucv.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/threads/combined/prulock.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/threads/combined/pruthr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/threads/combined/prustack.c"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/threads/prcmon.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/threads/prdump.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/threads/prmon.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/threads/prsem.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/threads/prrwlock.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/threads/prcthr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/threads/prtpd.c")

add_library(nspr ${LIBRARY_TYPE} ${NSPR_SRCS})
add_library(mozjs::nspr ALIAS nspr)

# disable all warnings
target_compile_options(nspr PRIVATE /W0)

target_include_directories(nspr PUBLIC
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/md"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/obsolete"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/private"
  "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(nspr PUBLIC advapi32 wsock32 ws2_32 winmm)

target_compile_definitions(nspr PRIVATE
  -D_NSPR_BUILD_
  -D_PR_GLOBAL_THREADS_ONLY
  -DXP_PC
  -DWIN95)

if(MOZJS_STATIC_LIB)
  target_compile_definitions(nspr PUBLIC -D_NSPR_STATIC_)
endif()

if(TARGET_CPU MATCHES "^x86_64$")
  target_compile_definitions(nspr PRIVATE
    -DUSE_64)
  message(STATUS "Build NSPR as 64-bit")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/md/_win95.cfg ${CMAKE_CURRENT_BINARY_DIR}/prcpucfg.h COPYONLY)
target_sources(nspr PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/prcpucfg.h)

if (MOZJS_INSTALL)
  install(TARGETS nspr
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${LIBRARY_DIR})
endif()

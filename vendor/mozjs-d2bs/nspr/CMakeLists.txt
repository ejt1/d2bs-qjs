if(CMAKE_SIZEOF_VOID_P MATCHES 8)
  add_definitions(-D_AMD64_)
else()
  add_definitions(-D_X86_)
endif()

set(NSPR_SRCS
  src/io/prdir.c
  src/io/prfile.c
  src/io/prio.c
  src/io/prfdcach.c
  src/io/prmwait.c
  src/io/priometh.c 
  src/io/pripv6.c
  src/io/prmapopt.c
  src/io/prlayer.c
  src/io/prlog.c
  src/io/prmmap.c
  src/io/prpolevt.c
  src/io/prprf.c
  src/io/prscanf.c
  src/io/prsocket.c
  src/io/prstdio.c

  src/linking/prlink.c
  
  src/malloc/prmalloc.c
  src/malloc/prmem.c

  src/md/windows/ntdllmn.c
  src/md/windows/ntmisc.c
  src/md/windows/ntsec.c
  src/md/windows/ntsem.c
  src/md/windows/ntinrval.c
  src/md/windows/ntgc.c
  src/md/windows/ntthread.c
  src/md/windows/ntio.c
  src/md/windows/win32_errors.c
  src/md/windows/w32ipcsem.c
  src/md/windows/w32poll.c
  src/md/windows/w32rng.c
  src/md/windows/w32shm.c
  src/md/prosdep.c
  
  src/memory/prseg.c
  src/memory/prshm.c
  src/memory/prshma.c

  src/misc/pralarm.c
  src/misc/pratom.c
  src/misc/prcountr.c
  src/misc/prdtoa.c
  src/misc/prenv.c
  src/misc/prerr.c
  src/misc/prerror.c
  src/misc/prerrortable.c
  src/misc/prinit.c
  src/misc/prinrval.c
  src/misc/pripc.c
  src/misc/pripcsem.c
  src/misc/prlog2.c
  src/misc/prlong.c
  src/misc/prnetdb.c
  src/misc/praton.c
  src/misc/prolock.c
  src/misc/prrng.c
  src/misc/prsystem.c
  src/misc/prtime.c
  src/misc/prthinfo.c
  src/misc/prtpool.c
  src/misc/prtrace.c

  src/threads/combined/prucpu.c
  src/threads/combined/prucv.c
  src/threads/combined/prulock.c
  src/threads/combined/pruthr.c
  src/threads/combined/prustack.c

  src/threads/prcmon.c
	src/threads/prdump.c
	src/threads/prmon.c
	src/threads/prsem.c
	src/threads/prrwlock.c
	src/threads/prcthr.c
	src/threads/prtpd.c)

add_library(nspr ${NSPR_SRCS})

target_include_directories(nspr
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/md
    ${CMAKE_CURRENT_SOURCE_DIR}/include/obsolete
    ${CMAKE_CURRENT_SOURCE_DIR}/include/private
  PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(nspr PUBLIC advapi32 wsock32 winmm)

target_compile_definitions(nspr PRIVATE -D_NSPR_BUILD_ PUBLIC -D_NSPR_STATIC_)

#target_compile_options(nspr PRIVATE /W0)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/md/_winnt.cfg ${CMAKE_CURRENT_BINARY_DIR}/prcpucfg.h COPYONLY)
target_sources(nspr PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/prcpucfg.h)

install(TARGETS nspr
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

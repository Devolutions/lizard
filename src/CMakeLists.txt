

set(LZMA_DIR "lzma")

include_directories(${LZMA_DIR})

if(WIN32)
	set(LZMA_THREADS 1)
else()
	add_definitions(-D_7ZIP_ST)
endif()

set(LZMA_HEADERS
	${LZMA_DIR}/7z.h
	${LZMA_DIR}/7zAlloc.h
	${LZMA_DIR}/7zBuf.h
	${LZMA_DIR}/7zCrc.h
	${LZMA_DIR}/7zFile.h
	${LZMA_DIR}/7zMemInStream.h
	${LZMA_DIR}/7zTypes.h
	${LZMA_DIR}/7zVersion.h
	${LZMA_DIR}/Aes.h
	${LZMA_DIR}/Alloc.h
	${LZMA_DIR}/Bcj2.h
	${LZMA_DIR}/Bra.h
	${LZMA_DIR}/Compiler.h
	${LZMA_DIR}/CpuArch.h
	${LZMA_DIR}/Delta.h
	${LZMA_DIR}/LzFind.h
	${LZMA_DIR}/LzHash.h
	${LZMA_DIR}/Lzma2Dec.h
	${LZMA_DIR}/Lzma2Enc.h
	${LZMA_DIR}/Lzma86.h
	${LZMA_DIR}/LzmaDec.h
	${LZMA_DIR}/LzmaEnc.h
	${LZMA_DIR}/LzmaLib.h
	${LZMA_DIR}/Ppmd.h
	${LZMA_DIR}/Ppmd7.h
	${LZMA_DIR}/Precomp.h
	${LZMA_DIR}/RotateDefs.h
	${LZMA_DIR}/Sha256.h
	${LZMA_DIR}/Sort.h
	${LZMA_DIR}/Xz.h
	${LZMA_DIR}/XzCrc64.h
	${LZMA_DIR}/XzEnc.h)

set(LZMA_SOURCES
	${LZMA_DIR}/7zAlloc.c
	${LZMA_DIR}/7zArcIn.c
	${LZMA_DIR}/7zBuf.c
	${LZMA_DIR}/7zBuf2.c
	${LZMA_DIR}/7zCrc.c
	${LZMA_DIR}/7zCrcOpt.c
	${LZMA_DIR}/7zDec.c
	${LZMA_DIR}/7zFile.c
	${LZMA_DIR}/7zMemInStream.c
	${LZMA_DIR}/7zStream.c
	${LZMA_DIR}/Aes.c
	${LZMA_DIR}/AesOpt.c
	${LZMA_DIR}/Alloc.c
	${LZMA_DIR}/Bcj2.c
	${LZMA_DIR}/Bcj2Enc.c
	${LZMA_DIR}/Bra.c
	${LZMA_DIR}/Bra86.c
	${LZMA_DIR}/BraIA64.c
	${LZMA_DIR}/CpuArch.c
	${LZMA_DIR}/Delta.c
	${LZMA_DIR}/LzFind.c
	${LZMA_DIR}/Lzma2Dec.c
	${LZMA_DIR}/Lzma2Enc.c
	${LZMA_DIR}/Lzma86Dec.c
	${LZMA_DIR}/Lzma86Enc.c
	${LZMA_DIR}/LzmaDec.c
	${LZMA_DIR}/LzmaEnc.c
	${LZMA_DIR}/LzmaLib.c
	${LZMA_DIR}/Ppmd7.c
	${LZMA_DIR}/Ppmd7Dec.c
	${LZMA_DIR}/Ppmd7Enc.c
	${LZMA_DIR}/Sha256.c
	${LZMA_DIR}/Sort.c
	${LZMA_DIR}/Xz.c
	${LZMA_DIR}/XzCrc64.c
	${LZMA_DIR}/XzCrc64Opt.c
	${LZMA_DIR}/XzDec.c
	${LZMA_DIR}/XzEnc.c
	${LZMA_DIR}/XzIn.c)

if(WIN32)
	list(APPEND LZMA_HEADERS ${LZMA_DIR}/DllSecur.h)
	list(APPEND LZMA_SOURCES ${LZMA_DIR}/DllSecur.c)
endif()

if(LZMA_THREADS)
	list(APPEND LZMA_HEADERS ${LZMA_DIR}/LzFindMt.h)
	list(APPEND LZMA_HEADERS ${LZMA_DIR}/MtCoder.h)
	list(APPEND LZMA_HEADERS ${LZMA_DIR}/Threads.h)

	list(APPEND LZMA_SOURCES ${LZMA_DIR}/LzFindMt.c)
	list(APPEND LZMA_SOURCES ${LZMA_DIR}/MtCoder.c)
	list(APPEND LZMA_SOURCES ${LZMA_DIR}/Threads.c)
endif()

set(LIZARD_HEADERS
	"")

set(LIZARD_SOURCES
	LzArchive.c
	LzEnv.c
	LzFile.c
	LzPath.c
	LzUnicode.c
	LzJson.c)

if(WIN32)
	list(APPEND LIZARD_SOURCES LzHttp.c)
	list(APPEND LIZARD_SOURCES LzWindows.c)
endif()

add_library(lizard STATIC
	${LIZARD_SOURCES} ${LIZARD_HEADERS}
	${LZMA_SOURCES} ${LZMA_HEADERS})

if(WIN32)
	target_link_libraries(lizard winhttp)
endif()

if(LIZARD_INSTALL)
	target_include_directories(lizard
		INTERFACE $<INSTALL_INTERFACE:include>)

	install(TARGETS lizard
		EXPORT lizard-target
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib)

	export(TARGETS lizard
		APPEND FILE ${PROJECT_BINARY_DIR}/lizard-target.cmake)

	install(EXPORT lizard-target
	        FILE lizard-target.cmake
	        DESTINATION "${CMAKE_EXPORT_DIR}")
endif()

cmake_minimum_required(VERSION 3.0)

project(xsf)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.1 -std=gnu11 -Wno-enum-compare")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1 -Wno-enum-compare")

add_definitions(-DM_CORE_GB)
add_definitions(-DM_CORE_GBA)
add_definitions(-DHAVE_CRC32)
add_definitions(-DHAVE_STRDUP)
add_definitions(-DHAVE_STRNDUP)
add_definitions(-DMINIMAL_CORE=3)
add_definitions(-DDISABLE_THREADING)
add_definitions(-DHAVE_INTTYPES_H)
add_definitions(-DHAVE_STDINT_H)
add_definitions(-DHAVE_STDBOOL_H)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mgba)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mgba/src)

set(HEADERS
    file2sf.h
    filegsf.h
    fileusf.h
    filencsf.h
    filesnsf.h
    filemsu.h
)

set(SOURCES
    file2sf.cpp
    filegsf.cpp
    fileusf.cpp
    filencsf.cpp
    filesnsf.cpp
    filemsu.cpp
    psflib/psf2fs.c
    psflib/psflib.c
    psflib/psfcore.cpp
    vio2sf/desmume/state.c
    vio2sf/desmume/NDSSystem.c
    vio2sf/desmume/MMU.c
    vio2sf/desmume/FIFO.c
    vio2sf/desmume/mc.c
    vio2sf/desmume/GPU.c
    vio2sf/desmume/armcpu.c
    vio2sf/desmume/arm_instructions.c
    vio2sf/desmume/bios.c
    vio2sf/desmume/thumb_instructions.c
    vio2sf/desmume/cp15.c
    vio2sf/desmume/SPU.cpp
    vio2sf/desmume/isqrt.c
    vio2sf/desmume/resampler.c
    vio2sf/desmume/barray.c
    mgba/src/core/core.c
    mgba/src/core/timing.c
    mgba/src/core/interface.c
    mgba/src/core/config.c
    mgba/src/core/sync.c
    mgba/src/core/log.c
    mgba/src/core/cheats.c
    mgba/src/core/cache-set.c
    mgba/src/core/map-cache.c
    mgba/src/core/tile-cache.c
    mgba/src/util/vfs/vfs-mem.c
    mgba/src/util/vfs/vfs-fd.c
    mgba/src/util/vfs.c
    mgba/src/util/configuration.c
    mgba/src/util/table.c
    mgba/src/util/hash.c
    mgba/src/util/formatting.c
    mgba/src/util/gbacrc32.c
    mgba/src/util/patch.c
    mgba/src/gba/gba.c
    mgba/src/gba/gbacore.c
    mgba/src/gba/gbaaudio.c
    mgba/src/gba/gbabios.c
    mgba/src/gba/hle-bios.c
    mgba/src/gba/gbamemory.c
    mgba/src/gba/dma.c
    mgba/src/gba/vfame.c
    mgba/src/gba/gbavideo.c
    mgba/src/gba/savedata.c
    mgba/src/gba/gbaio.c
    mgba/src/gba/gbasio.c
    mgba/src/gba/hardware.c
    mgba/src/gba/gbaoverrides.c
    mgba/src/gba/gbatimer.c
    mgba/src/gba/gbacheats.c
    mgba/src/gba/gbaserialize.c
    mgba/src/gba/cheats/gameshark.c
    mgba/src/gba/cheats/parv3.c
    mgba/src/gba/cheats/codebreaker.c
    mgba/src/gb/gb.c
    mgba/src/gb/gbcore.c
    mgba/src/gb/gbaudio.c
    mgba/src/gb/gbcheats.c
    mgba/src/gb/gbserialize.c
    mgba/src/gb/gbvideo.c
    mgba/src/gb/mbc.c
    mgba/src/gb/gbio.c
    mgba/src/gb/gbsio.c
    mgba/src/gb/gbmemory.c
    mgba/src/gb/gbtimer.c
    mgba/src/gb/gboverrides.c
    mgba/src/gb/renderers/software.c
    mgba/src/gb/renderers/rcache-set.c
    mgba/src/arm/arm.c
    mgba/src/arm/isa-thumb.c
    mgba/src/arm/isa-arm.c
    mgba/src/arm/decoder-arm.c
    mgba/src/arm/decoder-thumb.c
    mgba/src/third-party/blip_buf/blip_buf.c
    mgba/src/third-party/inih/ini.c
    mgba/src/util/circle-buffer.c
    mgba/src/util/string.c
    mgba/src/util/patch-ips.c
    mgba/src/util/patch-ups.c
    mgba/src/lr35902/lr35902.c
    mgba/src/lr35902/isa-lr35902.c
    n64/usf.c
    n64/memory.c
    n64/cpu.c
    n64/tlb.c
    n64/registers.c
    n64/interpreter_cpu.c
    n64/interpreter_ops.c
    n64/audio.c
    n64/exception.c
    n64/udma.c
    n64/main.c
    n64/pif.c
    n64/rsp/rsp.c
    n64/rsp_hle/hleaudio.c
    n64/rsp_hle/hle.c
    n64/rsp_hle/alist_audio.c
    n64/rsp_hle/alist_naudio.c
    n64/rsp_hle/alist_nead.c
    n64/rsp_hle/alist.c
    n64/rsp_hle/hlememory.c
    n64/rsp_hle/plugin.c
    n64/rsp_hle/jpeg.c
    n64/rsp_hle/cicx105.c
    n64/rsp_hle/musyx.c
    n64/rsp_hle/mp3.c
    sseq/Player.cpp
    sseq/Track.cpp
    sseq/Channel.cpp
    sseq/FATSection.cpp
    sseq/INFOEntry.cpp
    sseq/INFOSection.cpp
    sseq/SDAT.cpp
    sseq/NDSStdHeader.cpp
    sseq/SYMBSection.cpp
    sseq/SSEQ.cpp
    sseq/SBNK.cpp
    sseq/SWAR.cpp
    sseq/SWAV.cpp
    snes9x/SNESSystem.cpp
    snes9x/scpu.cpp
    snes9x/cpuexec.cpp
    snes9x/cpuops.cpp
    snes9x/sdma.cpp
    snes9x/globals.cpp
    snes9x/memmap.cpp
    snes9x/ppu.cpp
    snes9x/sa1.cpp
    snes9x/sdd1.cpp
    snes9x/apu/apu.cpp
    snes9x/apu/SNES_SPC.cpp
    snes9x/apu/SNES_SPC_misc.cpp
    snes9x/apu/SNES_SPC_state.cpp
    snes9x/apu/SPC_DSP.cpp
    snes9x/apu/SPC_Filter.cpp
    zlib/inflate.c
    zlib/uncompr.c
    zlib/crc32.c
    zlib/zutil.c
    zlib/adler32.c
    zlib/inftrees.c
    zlib/inffast.c
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}${STATIC_LIB_SUFFIX})

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    COMPONENT library
)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/lib${PROJECT_NAME})
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/psflib" DESTINATION "include/lib${PROJECT_NAME}" FILES_MATCHING PATTERN "*.h")
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/mgba/mgba/core" DESTINATION "include/lib${PROJECT_NAME}/mgba/mgba" FILES_MATCHING PATTERN "*.h")
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/mgba/mgba-util" DESTINATION "include/lib${PROJECT_NAME}/mgba" FILES_MATCHING PATTERN "*.h")

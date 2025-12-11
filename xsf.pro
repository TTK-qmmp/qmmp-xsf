
QMAKE_CFLAGS += -msse4.1 -std=gnu11
greaterThan(QT_MAJOR_VERSION, 5){
    QMAKE_CXXFLAGS += -msse4.1 -std=c++17
}else{
    QMAKE_CXXFLAGS += -msse4.1 -std=c++11
}

HEADERS += decoderxsffactory.h \
           decoder_xsf.h \
           xsfhelper.h \
           xsfmetadatamodel.h

SOURCES += decoderxsffactory.cpp \
           decoder_xsf.cpp \
           xsfhelper.cpp \
           xsfmetadatamodel.cpp \
           libxsf/file2sf.cpp \
           libxsf/filegsf.cpp \
           libxsf/fileusf.cpp \
           libxsf/filencsf.cpp \
           libxsf/filesnsf.cpp \
           libxsf/filemsu.cpp \
           libxsf/psflib/psf2fs.c \
           libxsf/psflib/psflib.c \
           libxsf/psflib/psfcore.cpp \
           libxsf/vio2sf/desmume/state.c \
           libxsf/vio2sf/desmume/NDSSystem.c \
           libxsf/vio2sf/desmume/MMU.c \
           libxsf/vio2sf/desmume/FIFO.c \
           libxsf/vio2sf/desmume/mc.c \
           libxsf/vio2sf/desmume/GPU.c \
           libxsf/vio2sf/desmume/armcpu.c \
           libxsf/vio2sf/desmume/arm_instructions.c \
           libxsf/vio2sf/desmume/bios.c \
           libxsf/vio2sf/desmume/thumb_instructions.c \
           libxsf/vio2sf/desmume/cp15.c \
           libxsf/vio2sf/desmume/SPU.cpp \
           libxsf/vio2sf/desmume/isqrt.c \
           libxsf/vio2sf/desmume/resampler.c \
           libxsf/vio2sf/desmume/barray.c \
           libxsf/mgba/src/core/core.c \
           libxsf/mgba/src/core/timing.c \
           libxsf/mgba/src/core/interface.c \
           libxsf/mgba/src/core/config.c \
           libxsf/mgba/src/core/sync.c \
           libxsf/mgba/src/core/log.c \
           libxsf/mgba/src/core/cheats.c \
           libxsf/mgba/src/core/cache-set.c \
           libxsf/mgba/src/core/map-cache.c \
           libxsf/mgba/src/core/tile-cache.c \
           libxsf/mgba/src/util/vfs/vfs-mem.c \
           libxsf/mgba/src/util/vfs/vfs-fd.c \
           libxsf/mgba/src/util/vfs.c \
           libxsf/mgba/src/util/configuration.c \
           libxsf/mgba/src/util/table.c \
           libxsf/mgba/src/util/hash.c \
           libxsf/mgba/src/util/formatting.c \
           libxsf/mgba/src/util/gbacrc32.c \
           libxsf/mgba/src/util/patch.c \
           libxsf/mgba/src/gba/gba.c \
           libxsf/mgba/src/gba/gbacore.c \
           libxsf/mgba/src/gba/gbaaudio.c \
           libxsf/mgba/src/gba/gbabios.c \
           libxsf/mgba/src/gba/hle-bios.c \
           libxsf/mgba/src/gba/gbamemory.c \
           libxsf/mgba/src/gba/dma.c \
           libxsf/mgba/src/gba/vfame.c \
           libxsf/mgba/src/gba/gbavideo.c \
           libxsf/mgba/src/gba/savedata.c \
           libxsf/mgba/src/gba/gbaio.c \
           libxsf/mgba/src/gba/gbasio.c \
           libxsf/mgba/src/gba/hardware.c \
           libxsf/mgba/src/gba/gbaoverrides.c \
           libxsf/mgba/src/gba/gbatimer.c \
           libxsf/mgba/src/gba/gbacheats.c \
           libxsf/mgba/src/gba/gbaserialize.c \
           libxsf/mgba/src/gba/cheats/gameshark.c \
           libxsf/mgba/src/gba/cheats/parv3.c \
           libxsf/mgba/src/gba/cheats/codebreaker.c \
           libxsf/mgba/src/gb/gb.c \
           libxsf/mgba/src/gb/gbcore.c \
           libxsf/mgba/src/gb/gbaudio.c \
           libxsf/mgba/src/gb/gbcheats.c \
           libxsf/mgba/src/gb/gbserialize.c \
           libxsf/mgba/src/gb/gbvideo.c \
           libxsf/mgba/src/gb/mbc.c \
           libxsf/mgba/src/gb/gbio.c \
           libxsf/mgba/src/gb/gbsio.c \
           libxsf/mgba/src/gb/gbmemory.c \
           libxsf/mgba/src/gb/gbtimer.c \
           libxsf/mgba/src/gb/gboverrides.c \
           libxsf/mgba/src/gb/renderers/software.c \
           libxsf/mgba/src/gb/renderers/rcache-set.c \
           libxsf/mgba/src/arm/arm.c \
           libxsf/mgba/src/arm/isa-thumb.c \
           libxsf/mgba/src/arm/isa-arm.c \
           libxsf/mgba/src/arm/decoder-arm.c \
           libxsf/mgba/src/arm/decoder-thumb.c \
           libxsf/mgba/src/third-party/blip_buf/blip_buf.c \
           libxsf/mgba/src/third-party/inih/ini.c \
           libxsf/mgba/src/util/circle-buffer.c \
           libxsf/mgba/src/util/string.c \
           libxsf/mgba/src/util/patch-ips.c \
           libxsf/mgba/src/util/patch-ups.c \
           libxsf/mgba/src/lr35902/lr35902.c \
           libxsf/mgba/src/lr35902/isa-lr35902.c \
           libxsf/n64/usf.c \
           libxsf/n64/memory.c \
           libxsf/n64/cpu.c \
           libxsf/n64/tlb.c \
           libxsf/n64/registers.c \
           libxsf/n64/interpreter_cpu.c \
           libxsf/n64/interpreter_ops.c \
           libxsf/n64/audio.c \
           libxsf/n64/exception.c \
           libxsf/n64/udma.c \
           libxsf/n64/main.c \
           libxsf/n64/pif.c \
           libxsf/n64/rsp/rsp.c \
           libxsf/n64/rsp_hle/hleaudio.c \
           libxsf/n64/rsp_hle/hle.c \
           libxsf/n64/rsp_hle/alist_audio.c \
           libxsf/n64/rsp_hle/alist_naudio.c \
           libxsf/n64/rsp_hle/alist_nead.c \
           libxsf/n64/rsp_hle/alist.c \
           libxsf/n64/rsp_hle/hlememory.c \
           libxsf/n64/rsp_hle/plugin.c \
           libxsf/n64/rsp_hle/jpeg.c \
           libxsf/n64/rsp_hle/cicx105.c \
           libxsf/n64/rsp_hle/musyx.c \
           libxsf/n64/rsp_hle/mp3.c \
           libxsf/sseq/Player.cpp \
           libxsf/sseq/Track.cpp \
           libxsf/sseq/Channel.cpp \
           libxsf/sseq/FATSection.cpp \
           libxsf/sseq/INFOEntry.cpp \
           libxsf/sseq/INFOSection.cpp \
           libxsf/sseq/SDAT.cpp \
           libxsf/sseq/NDSStdHeader.cpp \
           libxsf/sseq/SYMBSection.cpp \
           libxsf/sseq/SSEQ.cpp \
           libxsf/sseq/SBNK.cpp \
           libxsf/sseq/SWAR.cpp \
           libxsf/sseq/SWAV.cpp \
           libxsf/snes9x/SNESSystem.cpp \
           libxsf/snes9x/scpu.cpp \
           libxsf/snes9x/cpuexec.cpp \
           libxsf/snes9x/cpuops.cpp \
           libxsf/snes9x/sdma.cpp \
           libxsf/snes9x/globals.cpp \
           libxsf/snes9x/memmap.cpp \
           libxsf/snes9x/ppu.cpp \
           libxsf/snes9x/sa1.cpp \
           libxsf/snes9x/sdd1.cpp \
           libxsf/snes9x/apu/apu.cpp \
           libxsf/snes9x/apu/SNES_SPC.cpp \
           libxsf/snes9x/apu/SNES_SPC_misc.cpp \
           libxsf/snes9x/apu/SNES_SPC_state.cpp \
           libxsf/snes9x/apu/SPC_DSP.cpp \
           libxsf/snes9x/apu/SPC_Filter.cpp \
           libxsf/zlib/inflate.c \
           libxsf/zlib/uncompr.c \
           libxsf/zlib/zutil.c \
           libxsf/zlib/crc32.c \
           libxsf/zlib/adler32.c \
           libxsf/zlib/inftrees.c \
           libxsf/zlib/inffast.c

DEFINES += M_CORE_GB \
           M_CORE_GBA \
           HAVE_CRC32 \
           HAVE_STRDUP \
           HAVE_STRNDUP \
           MINIMAL_CORE=3 \
           DISABLE_THREADING \
           HAVE_INTTYPES_H \
           HAVE_STDINT_H \
           HAVE_STDBOOL_H

INCLUDEPATH += $$PWD/libxsf/mgba \
               $$PWD/libxsf/mgba/src

#CONFIG += BUILD_PLUGIN_INSIDE
contains(CONFIG, BUILD_PLUGIN_INSIDE){
    include($$PWD/../../plugins.pri)
    TARGET = $$PLUGINS_PREFIX/Input/xsf

    unix{
        target.path = $$PLUGIN_DIR/Input
        INSTALLS += target
    }
}else{
    QT += widgets
    CONFIG += warn_off plugin lib thread link_pkgconfig c++11
    TEMPLATE = lib

    unix{
        equals(QT_MAJOR_VERSION, 4){
            QMMP_PKG = qmmp-0
        }else:equals(QT_MAJOR_VERSION, 5){
            QMMP_PKG = qmmp-1
        }else:equals(QT_MAJOR_VERSION, 6){
            QMMP_PKG = qmmp
        }else{
            error("No Qt version found")
        }
        
        PKGCONFIG += $${QMMP_PKG}

        PLUGIN_DIR = $$system(pkg-config $${QMMP_PKG} --variable=plugindir)/Input
        INCLUDEPATH += $$system(pkg-config $${QMMP_PKG} --variable=prefix)/include

        plugin.path = $${PLUGIN_DIR}
        plugin.files = lib$${TARGET}.so
        INSTALLS += plugin
    }
}

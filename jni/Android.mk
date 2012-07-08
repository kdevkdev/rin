LOCAL_PATH := $(call my-dir)

MY_LIBRARY_TYPE:=static

$(warning Value of TARGET_ARCH_ABI '$(TARGET_ARCH_ABI)') 

ifeq ($(TARGET_ARCH_ABI),armeabi)
	MY_LPREF:=arm
endif
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
	MY_LPREF:=arm
endif
ifeq ($(TARGET_ARCH_ABI),mips)
	MY_LPREF:=mips
endif
ifeq ($(TARGET_ARCH_ABI),x86)
	MY_LPREF:=x86
endif

$(warning Value of LPREF '$(MY_LPREF)') 

ifeq ($(MY_LIBRARY_TYPE),shared)

#	include $(CLEAR_VARS)
#	LOCAL_MODULE := libpng
#	LOCAL_SRC_FILES := libs/$(MY_LPREF)/libpng14.so
#	include $(PREBUILT_SHARED_LIBRARY)

	include $(CLEAR_VARS)
	LOCAL_MODULE := liblodepng
	LOCAL_SRC_FILES := libs/$(MY_LPREF)/liblodepng.so
	include $(PREBUILT_SHARED_LIBRARY)

#	include $(CLEAR_VARS)
#	LOCAL_MODULE := libz
#	LOCAL_SRC_FILES := libs/$(MY_LPREF)/libz.so
#	include $(PREBUILT_SHARED_LIBRARY)

#	include $(CLEAR_VARS)
#	LOCAL_MODULE := unziplib
#	LOCAL_SRC_FILES := libs/$(MY_LPREF)/unziplib.so
#	include $(PREBUILT_SHARED_LIBRARY)

else

#	include $(CLEAR_VARS)
#	LOCAL_MODULE := libpng
#	LOCAL_SRC_FILES := libs/$(MY_LPREF)/libpng.a
#	include $(PREBUILT_STATIC_LIBRARY)

	include $(CLEAR_VARS)
	LOCAL_MODULE := liblodepng
	LOCAL_SRC_FILES := libs/$(MY_LPREF)/liblodepng.a
	include $(PREBUILT_STATIC_LIBRARY)

#	include $(CLEAR_VARS)
#	LOCAL_MODULE := libz
#	LOCAL_SRC_FILES := libs/$(MY_LPREF)/libz.a
#	include $(PREBUILT_STATIC_LIBRARY)

#	include $(CLEAR_VARS)
#	LOCAL_MODULE := unziplib
#	LOCAL_SRC_FILES := libs/$(MY_LPREF)/unziplib.a
#	include $(PREBUILT_STATIC_LIBRARY)

endif


include $(CLEAR_VARS)


ifeq ($(TARGET_ARCH_ABI),armeabi)
	
endif
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
	#build additional library for neon and later hackily load it with java if neon supported (with the help of libpreloadcheck)
	LOCAL_ARM_NEON  := true
	LOCAL_MODULE    := rin-neon
	LOCAL_LDLIBS	:=-ljnigraphics -lz
	LOCAL_SRC_FILES := queue.c order.c rin.c pg.c graphics.c sound.c gb/gb.c saveload.c gb/cheat.c gb/cpu.c gb/mbc.c gb/sgb.c gb/rom.c filer.c gb/lcd.c renderer.c  gb/apu.c rewind.c


	ifeq ($(MY_LIBRARY_TYPE),shared)
		LOCAL_SHARED_LIBRARIES := liblodepng
	else
		LOCAL_STATIC_LIBRARIES := liblodepng
	endif
	include $(BUILD_SHARED_LIBRARY)
endif
ifeq ($(TARGET_ARCH_ABI),mips)
	
endif
ifeq ($(TARGET_ARCH_ABI),x86)
	
endif

include $(CLEAR_VARS)

LOCAL_ARM_NEON  := false
LOCAL_MODULE    := rin
LOCAL_LDLIBS	:= -ljnigraphics -lz
LOCAL_SRC_FILES := queue.c order.c rin.c pg.c graphics.c sound.c gb/gb.c saveload.c gb/cheat.c gb/cpu.c gb/mbc.c gb/sgb.c gb/rom.c filer.c gb/lcd.c renderer.c  gb/apu.c rewind.c


ifeq ($(MY_LIBRARY_TYPE),shared)
	LOCAL_SHARED_LIBRARIES := liblodepng
else
	LOCAL_STATIC_LIBRARIES := liblodepng
endif


include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE :=preloadcheck
LOCAL_SRC_FILES:=preloadcheck.c
LOCAL_STATIC_LIBRARIES := cpufeatures
include $(BUILD_SHARED_LIBRARY)
$(call import-module,android/cpufeatures)



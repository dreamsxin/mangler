ROOT := $(call my-dir)

# Build libgsm
include $(CLEAR_VARS)
LOCAL_PATH			:= $(ROOT)/gsm/src
LOCAL_MODULE		:= libgsm
LOCAL_SRC_FILES		:= add.c			code.c				debug.c				decode.c \
					   long_term.c		lpc.c				preprocess.c		rpe.c \
				       gsm_destroy.c	gsm_decode.c		gsm_encode.c		gsm_explode.c \
				       gsm_implode.c	gsm_create.c		gsm_print.c			gsm_option.c \
				       short_term.c		table.c
LOCAL_CFLAGS		:= -I$(shell cygpath -m $(LOCAL_PATH)/../inc) -DWAV49 -O3
include $(BUILD_STATIC_LIBRARY)

# Build libspeex
include $(CLEAR_VARS)
LOCAL_PATH 			:= $(ROOT)/speex/libspeex
LOCAL_MODULE		:= libspeex
LOCAL_SRC_FILES		:= cb_search.c		exc_10_32_table.c 	exc_8_128_table.c 	filters.c \
					   gain_table.c 	hexc_table.c 		high_lsp_tables.c 	lsp.c \
					   ltp.c			speex.c 			stereo.c 			vbr.c \
					   vq.c bits.c 		exc_10_16_table.c	exc_20_32_table.c 	exc_5_256_table.c \
					   exc_5_64_table.c	gain_table_lbr.c 	hexc_10_32_table.c	lpc.c \
					   lsp_tables_nb.c 	modes.c 			modes_wb.c 			nb_celp.c \
					   quant_lsp.c		sb_celp.c			speex_callbacks.c 	speex_header.c \
					   window.c			resample.c
LOCAL_CFLAGS		:= -I$(shell cygpath -m $(LOCAL_PATH)/../include) -D__EMX__ -DFIXED_POINT -DEXPORT='' -O3
include $(BUILD_STATIC_LIBRARY)

# Build libcelt
include $(CLEAR_VARS)
LOCAL_PATH			:= $(ROOT)/celt/src
LOCAL_MODULE		:= libcelt
LOCAL_SRC_FILES		:= bands.c			celt.c				cwrs.c				dump_modes.c \
					   entcode.c		entdec.c			entenc.c			header.c \
					   kiss_fft.c		laplace.c			mdct.c				modes.c \
					   pitch.c			quant_bands.c		rangedec.c			rangeenc.c \
					   rate.c			testcelt.c			vq.c
LOCAL_CFLAGS		:= -I$(shell cygpath -m $(LOCAL_PATH)/../inc/celt) -Drestrict='' -D__EMX__ -DFIXED_POINT -DHAVE_LRINTF -DHAVE_LRINT -DDOUBLE_PRECISION -O3
include $(BUILD_STATIC_LIBRARY)

# Build libventrilo
include $(CLEAR_VARS)
LOCAL_PATH				:= $(LIBPATH)
LOCAL_MODULE    		:= libventrilo3
LOCAL_SRC_FILES 		:= libventrilo3.c libventrilo3_message.c ventrilo3_handshake.c
LOCAL_CFLAGS			:= -DANDROID -D__EMX__ -I$(shell cygpath -m $(ROOT)/gsm/inc) -I$(shell cygpath -m $(ROOT)/speex/include) -I$(shell cygpath -m $(ROOT)/celt/inc) -DHAVE_GSM -DHAVE_GSM_H -DHAVE_SPEEX -DHAVE_SPEEX_DSP=1 -DHAVE_CELT -DNO_AUTOMAKE -O3
include $(BUILD_STATIC_LIBRARY)

# Build library interface
include $(CLEAR_VARS)
LOCAL_PATH				:= $(ROOT)/ventrilo
LOCAL_MODULE    		:= libventrilo_interface
LOCAL_SRC_FILES 		:= jni_wrappers.c
LOCAL_LDLIBS			:= -llog
LOCAL_CFLAGS			:= -I$(shell cygpath -m $(LIBPATH)) -O3
LOCAL_STATIC_LIBRARIES	:= libventrilo3 libgsm libspeex libcelt
include $(BUILD_SHARED_LIBRARY)

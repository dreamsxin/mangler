# Both the __EMX__ and EXPORT definitions are to satisfy libspeex for building without autoconf.
ROOT := $(call my-dir)

# Build libgsm
include $(CLEAR_VARS)
LOCAL_PATH				:= $(ROOT)/gsm/src
LOCAL_MODULE			:= libgsm
LOCAL_SRC_FILES			:= add.c		code.c			debug.c			decode.c		long_term.c	\
						   lpc.c		preprocess.c	rpe.c			gsm_destroy.c	gsm_decode.c \
						   gsm_encode.c	gsm_explode.c	gsm_implode.c	gsm_create.c	gsm_print.c \
						   gsm_option.c	short_term.c	table.c
LOCAL_CFLAGS			:= -I$(LOCAL_PATH)/../inc
include $(BUILD_STATIC_LIBRARY)

# Build libspeex
include $(CLEAR_VARS)
LOCAL_PATH 				:= $(ROOT)/speex/libspeex
LOCAL_MODULE			:= libspeex
LOCAL_SRC_FILES			:= cb_search.c		exc_10_32_table.c 	exc_8_128_table.c 	filters.c \
						   gain_table.c 	hexc_table.c 		high_lsp_tables.c 	lsp.c \
						   ltp.c			speex.c 			stereo.c 			vbr.c \
						   vq.c bits.c 		exc_10_16_table.c	exc_20_32_table.c 	exc_5_256_table.c \
						   exc_5_64_table.c gain_table_lbr.c 	hexc_10_32_table.c	lpc.c \
						   lsp_tables_nb.c 	modes.c 			modes_wb.c 			nb_celp.c \
						   quant_lsp.c		sb_celp.c			speex_callbacks.c 	speex_header.c \
						   window.c
LOCAL_CFLAGS			:= -I$(LOCAL_PATH)/../include -D__EMX__ -DFIXED_POINT -DEXPORT=''
include $(BUILD_STATIC_LIBRARY)

# Build libventrilo
include $(CLEAR_VARS)
LOCAL_PATH 				:= $(ROOT)/ventrilo
LOCAL_MODULE    		:= libventrilo3
LOCAL_SRC_FILES 		:= libventrilo3.c libventrilo3_message.c ventrilo3_handshake.c jni_wrappers.c
LOCAL_CFLAGS			:= -DANDROID -D__EMX__ -fpack-struct=1 -I$(ROOT)/gsm/inc -I$(ROOT)/speex/include
LOCAL_LDLIBS			:= -llog
LOCAL_STATIC_LIBRARIES 	:= libgsm libspeex
include $(BUILD_SHARED_LIBRARY)

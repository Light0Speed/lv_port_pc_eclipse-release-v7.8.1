#
# Makefile
#
ifeq ($(origin CC), default)
CC := gcc
endif
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= ${shell pwd}

COMMON_WARNINGS ?= -Werror -Wall -Wextra \
						-Wshadow -Wundef -Wmissing-prototypes \
						-Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized \
						-Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default  \
					  -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wdeprecated  \
						-Wempty-body -Wshift-negative-value \
            -Wtype-limits -Wsizeof-pointer-memaccess -Wpointer-arith

GCC_WARNINGS ?= -Wmaybe-uninitialized -Wno-discarded-qualifiers -Wclobbered -Wstack-usage=2048 -Wno-error=maybe-uninitialized -Wno-error=cast-function-type

CLANG_WARNINGS ?=

ifeq ($(findstring clang,$(shell $(CC) --version 2>/dev/null)),clang)
WARNINGS ?= $(COMMON_WARNINGS) $(CLANG_WARNINGS)
else
WARNINGS ?= $(COMMON_WARNINGS) $(GCC_WARNINGS)
endif
            
CFLAGS ?= -O3 -g0 -I$(LVGL_DIR)/ $(WARNINGS)
LDFLAGS ?= -lSDL2 -lm
BIN = demo


#Collect the files to compile
MAINSRC = ./main.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk
include $(LVGL_DIR)/lv_examples/lv_examples.mk

CSRCS +=$(LVGL_DIR)/mouse_cursor_icon.c
CSRCS +=$(LVGL_DIR)/huge_list.c
CSRCS +=$(LVGL_DIR)/huge_list_demo.c

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

## MAINOBJ -> OBJFILES

all: default

%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"
    
default: $(AOBJS) $(COBJS) $(MAINOBJ)
	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)

clean: 
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ)


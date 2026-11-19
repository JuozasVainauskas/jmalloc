CC        = gcc
BUILD     ?= debug
# UBSAN=1 enables UBSan (misaligned access, integer overflow, bad shifts), make UBSAN=1 test
UBSAN     ?=
BUILDDIR  = build/$(BUILD)$(if $(UBSAN),-ubsan)

CPPFLAGS = -Iinclude -D_DEFAULT_SOURCE -MMD -MP
CFLAGS   = -std=c23 -Wall -Wextra -Wpedantic -pthread -fPIC -fno-builtin
LDFLAGS  = -pthread

CFLAGS_debug   = -O0 -g3
CFLAGS_release = -O2 -g -DNDEBUG
CFLAGS += $(CFLAGS_$(BUILD))

ifneq ($(UBSAN),)
CFLAGS  += -fsanitize=undefined -fno-omit-frame-pointer
LDFLAGS += -fsanitize=undefined
endif

LIB_OBJS := $(patsubst %.c,$(BUILDDIR)/%.o,$(wildcard src/*.c))
PROGRAMS := $(patsubst %.c,$(BUILDDIR)/%,$(wildcard tests/*.c examples/*.c))
TESTS    := $(filter $(BUILDDIR)/tests/%,$(PROGRAMS))

STATIC_LIB := $(BUILDDIR)/libjmalloc.a
SHARED_LIB := $(BUILDDIR)/libjmalloc.so

.PHONY: all build test run clean

all: $(STATIC_LIB) $(SHARED_LIB) $(PROGRAMS)
build: all

test: $(TESTS)
	@for t in $(TESTS); do echo "== $$t"; ./$$t || exit 1; done

EXAMPLE ?= main
run: $(BUILDDIR)/examples/$(EXAMPLE)
	./$<

clean:
	rm -rf build

$(STATIC_LIB): $(LIB_OBJS)
	@rm -f $@
	$(AR) rcs $@ $^

$(SHARED_LIB): $(LIB_OBJS)
	$(CC) -shared $(LDFLAGS) $^ -o $@

$(PROGRAMS): $(BUILDDIR)/%: $(BUILDDIR)/%.o $(STATIC_LIB)
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

-include $(LIB_OBJS:.o=.d) $(PROGRAMS:=.d)

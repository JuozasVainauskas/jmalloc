CC    = gcc
BUILD ?= debug
SAN   ?=

BUILDDIR = build/$(BUILD)$(if $(SAN),-$(SAN))

CPPFLAGS = -Iinclude -D_DEFAULT_SOURCE -MMD -MP
CFLAGS   = -std=c23 -Wall -Wextra -Wpedantic -pthread
LDFLAGS  = -pthread

CFLAGS_debug   = -O0 -g3
CFLAGS_release = -O2 -g -DNDEBUG
CFLAGS  += $(CFLAGS_$(BUILD)) $(EXTRA_CFLAGS)

ifneq ($(SAN),)
CFLAGS  += -fsanitize=$(SAN) -fno-omit-frame-pointer
LDFLAGS += -fsanitize=$(SAN)
endif

LIB_SRCS  := $(wildcard src/*.c)
TEST_SRCS := $(wildcard tests/*.c)
EX_SRCS   := $(wildcard examples/*.c)

LIB_OBJS  := $(LIB_SRCS:%.c=$(BUILDDIR)/%.o)
TEST_OBJS := $(TEST_SRCS:%.c=$(BUILDDIR)/%.o)
TEST_BINS := $(TEST_SRCS:tests/%.c=$(BUILDDIR)/tests/%)
EX_OBJS   := $(EX_SRCS:%.c=$(BUILDDIR)/%.o)
EX_BINS   := $(EX_SRCS:examples/%.c=$(BUILDDIR)/examples/%)
LIB       := $(BUILDDIR)/libjmalloc.a
PIC_OBJS  := $(LIB_SRCS:%.c=$(BUILDDIR)/pic/%.o)
SO        := $(BUILDDIR)/libjmalloc.so

EXAMPLE ?= main

.PHONY: all build run test clean

all: $(LIB) $(SO) $(TEST_BINS) $(EX_BINS)

build: all

$(LIB_OBJS) $(PIC_OBJS): CFLAGS += -fno-builtin

$(LIB): $(LIB_OBJS)
	$(AR) rcs $@ $^

$(SO): $(PIC_OBJS)
	$(CC) -shared $(LDFLAGS) $^ $(LDLIBS) -o $@

$(PIC_OBJS): $(BUILDDIR)/pic/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -c $< -o $@

$(TEST_BINS): $(BUILDDIR)/tests/%: $(BUILDDIR)/tests/%.o $(LIB)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EX_BINS): $(BUILDDIR)/examples/%: $(BUILDDIR)/examples/%.o $(LIB)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

test: $(TEST_BINS)
	@for t in $(TEST_BINS); do echo "== $$t"; ./$$t || exit 1; done

run: $(BUILDDIR)/examples/$(EXAMPLE)
	./$<

clean:
	rm -rf build

-include $(LIB_OBJS:.o=.d) $(PIC_OBJS:.o=.d) $(TEST_OBJS:.o=.d) $(EX_OBJS:.o=.d)

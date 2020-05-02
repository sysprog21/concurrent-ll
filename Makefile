# Configurable options
# MODE = release | debug (default: release)

# Management PC specific settings
OS_NAME := $(shell uname -s)
ifeq ($(OS_NAME),Darwin) # for OS X, use build in tools
CORE_NUM := $(shell sysctl -n hw.ncpu)
else # Linux and other
CORE_NUM := $(shell nproc)
endif

ifneq ($(CORE_SPEED_KHz), )
CFLAGS += -DCORE_NUM=${CORE_NUM}
else
CFLAGS += -DCORE_NUM=4
endif
CFLAGS += -DDEFAULT
$(info *** Using as a default number of cores: $(CORE_NUM) on 1 socket)
$(info ***)

# Generic configurations
CFLAGS += --std=gnu99 -pedantic -Wall
CFLAGS += -fno-strict-aliasing
CFLAGS += -D_GNU_SOURCE
CFLAGS += -D_REENTRANT
CFLAGS += -I include
LDFLAGS += -lpthread

ifneq ($(MODE),debug)
	CFLAGS += -O3 -DNDEBUG
else
	CFLAGS += -g
endif

OUT = out
EXEC = $(OUT)/test-lock $(OUT)/test-lockfree
all: $(EXEC)

deps =

LOCK_OBJS =
LOCK_OBJS += \
    src/lock/list.o \
    src/lock/main.o
deps += $(LOCK_OBJS:%.o=%.o.d)

$(OUT)/test-lock: $(LOCK_OBJS)
	@mkdir -p $(OUT)
	$(CC) -o $@ $^ $(LDFLAGS)
src/lock/%.o: src/lock/%.c
	$(CC) $(CFLAGS) -DLOCK_BASED -o $@ -MMD -MF $@.d -c $<

LOCKFREE_OBJS =
LOCKFREE_OBJS += \
    src/lockfree/list.o \
    src/lockfree/main.o
deps += $(LOCKFREE_OBJS:%.o=%.o.d)

$(OUT)/test-lockfree: $(LOCKFREE_OBJS)
	@mkdir -p $(OUT)
	$(CC) -o $@ $^ $(LDFLAGS)
src/lockfree/%.o: src/lockfree/%.c
	$(CC) $(CFLAGS) -DLOCKFREE -o $@ -MMD -MF $@.d -c $<

check: $(EXEC)
	bash scripts/test_correctness.sh

bench: $(EXEC)
	bash scripts/run_ll.sh
	bash scripts/create_plots_ll.sh >/dev/null
	@echo Check the plots generated in directory 'out/plots'.

clean:
	$(RM) -f $(EXEC)
	$(RM) -f $(LOCK_OBJS) $(LOCKFREE_OBJS) $(deps)

distclean: clean
	$(RM) -rf out

.PHONY: all check clean distclean

-include $(deps)

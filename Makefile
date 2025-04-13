OUT?=ddb_crystalizer.so

PKG_CFLAGS?=`pkg-config --cflags`
PKG_LIBS?=`pkg-config --libs`

CC?=gcc
CFLAGS+=-Wall -g -fPIC -std=c99 -D_GNU_SOURCE
LDFLAGS+=-shared

OUT_DIR?=out

SOURCES?=$(wildcard *.c)
OBJS?=$(patsubst %.c, $(OUT_DIR)/%.o, $(SOURCES))

define compile
	$(CC) $(CFLAGS) $1 $2 $< -c -o $@
endef

define link
	$(CC) $(LDFLAGS) $1 $2 $3 -o $@
endef

all: out

out: mkdir_out $(SOURCES) $(OUT_DIR)/$(OUT)

mkdir_out:
	@echo "Creating build directory"
	@mkdir -p $(OUT_DIR)

$(OUT_DIR)/$(OUT): $(OBJS)
	@echo "Linking"
	@$(call link, $(OBJS), $(LIBS))
	@echo "Done!"

$(OUT_DIR)/%.o: %.c
	@echo "Compiling $(subst $(OUT_DIR)/,,$@)"
	@$(call compile, $(PKG_CFLAGS))

clean:
	@echo "Cleaning files from previous build..."
	@rm -r -f $(OUT_DIR)

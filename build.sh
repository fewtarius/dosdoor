#!/bin/bash
# dosdoor build
set -e

OS=$(uname -s)
ARCH=$(uname -m)
echo "=== dosdoor build ==="
echo "Platform: ${OS} ${ARCH}"

if [ "$OS" = "Darwin" ]; then
    if [ ! -f Makefile.conf ]; then
        cp Makefile.conf.darwin Makefile.conf
        sed -i '' "s|abs_top_srcdir:=.*|abs_top_srcdir:=$(pwd)|" Makefile.conf
        # apply prefix if provided
        if [ -n "${PREFIX:-}" ]; then
            sed -i '' "s|^prefix:=.*|prefix:=${PREFIX}|" Makefile.conf
        fi
    fi
    [ -f config.status ] || { echo '#!/bin/sh' > config.status; chmod +x config.status; }
    # enable plugins (normally done by configure via mkpluginhooks)
    for d in kbd_unicode commands translate term; do
        mkdir -p src/plugin/$d/config
        echo "yes" > src/plugin/$d/config/plugin_enable
    done
    mkdir -p src/plugin/translate/charsets/config
    echo "yes" > src/plugin/translate/charsets/config/plugin_enable
else
    if [ ! -f Makefile.conf ]; then
        # pre-generate kbd_unicode_config.h so mkpluginhooks can symlink it
        mkdir -p src/plugin/kbd_unicode/include
        cat > src/plugin/kbd_unicode/include/kbd_unicode_config.h <<'KBDEOF'
#ifndef UNICODE_KEYB_CONFIG_H
#define UNICODE_KEYB_CONFIG_H
#define HAVE_UNICODE_KEYB 2
#define STDC_HEADERS 1
#endif
KBDEOF
        CONFIGURE_PREFIX="${PREFIX:-/usr}"
        export DOSEMU_DEFAULT_CONFIGURE=1
        ./configure --prefix="${CONFIGURE_PREFIX}" \
            --sysconfdir="${CONFIGURE_PREFIX}/etc/dosdoor" \
            --enable-cpuemu --disable-net --without-x --disable-sbemu \
            --disable-mitshm --without-vidmode --disable-aspi \
            --without-gpm --without-alsa --without-sndfile \
            --disable-dlplugins
        sed -i 's/CFLAGS += $(XXXCFLAGS)/CFLAGS += -fgnu89-inline $(XXXCFLAGS)/' Makefile.conf
    fi
fi

make "$@"

if [ -f build/bin/dosdoor ]; then
    echo ""
    echo "=== dosdoor build complete ==="
    file build/bin/dosdoor
fi

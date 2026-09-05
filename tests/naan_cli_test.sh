#!/bin/sh
set -eu

naan=$1

eq() {
    got=$1
    want=$2
    title=$3
    if [ "$got" != "$want" ]; then
        printf 'FAIL %s: got %s want %s\n' "$title" "$got" "$want" >&2
        exit 1
    fi
}

eq "$("$naan" hello)" 38948 "default hello"
eq "$("$naan" -2 hello)" 38948 "sha256 hello"
eq "$("$naan" -5 hello)" 50578 "md5 hello"
eq "$("$naan" -1 hello)" 17229 "sha1 hello"
eq "$("$naan" --crc16 hello)" 53870 "crc16 hello"
eq "$("$naan" --crc32 hello)" 42630 "crc32 hello"
eq "$("$naan" -c hello)" 42630 "crc32 short"
eq "$("$naan" --sha512 hello)" 49219 "sha512 hello"
eq "$("$naan" --sha3-256 hello)" 62354 "sha3-256 hello"
eq "$("$naan" -p b hello)" 36 "profile b"
eq "$("$naan" -p dw hello)" 2475399204 "profile dw"
eq "$("$naan" -p wm hello)" 3181 "profile wm"
eq "$("$naan" -m 1048576 hello)" 759844 "mod 1M"
eq "$("$naan" -5 -p wm world)" 3603 "sha1 after md5 then wm"
eq "$("$naan" -p wm -5 world)" 3351 "md5 overrides wm hash"

got=$("$naan" hello world)
want=$(printf '%s\n' 38948 47271)
eq "$got" "$want" "two names"

if "$naan" >/dev/null 2>&1; then
    echo "FAIL missing name should fail" >&2
    exit 1
fi

if "$naan" -p no-such hello >/dev/null 2>&1; then
    echo "FAIL unknown profile should fail" >&2
    exit 1
fi

if "$naan" -m 0 hello >/dev/null 2>&1; then
    echo "FAIL DIV=0 should fail" >&2
    exit 1
fi

eq "$("$naan" -m 10 -b 10 hello)" 10 "div equals bias"

"$naan" --help >/dev/null
"$naan" --version >/dev/null

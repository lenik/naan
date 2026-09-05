# bash completion for naan

_naan() {
    local cur prev
    COMPREPLY=()
    cur=${COMP_WORDS[COMP_CWORD]}
    prev=${COMP_WORDS[COMP_CWORD - 1]}

    case $prev in
    -p | --profile)
        COMPREPLY=($(compgen -W "b w dw wm" -- "$cur"))
        return
        ;;
    -m | --mod | -b | --bias)
        return
        ;;
    esac

    if [[ $cur == -* ]]; then
        COMPREPLY=($(compgen -W \
            "-5 --md5 -1 --sha1 --sha224 -2 --sha256 --sha384 -8 --sha512 -3 --sha3-256 --sha3-512 --blake2s --blake2b --ripemd160 --crc16 -c --crc32 -m --mod -b --bias -p --profile -h --help --version" \
            -- "$cur"))
    fi
}

complete -F _naan naan

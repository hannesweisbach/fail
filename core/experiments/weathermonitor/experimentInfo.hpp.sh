#!/bin/bash
set -e

[ ! -e "$1" -o ! -e "$2" ] && echo "usage: $0 vanilla.elf guarded.elf" && exit 1

function addrof() { nm -C $1 | fgrep "$2" | awk '{print $1}'; }

cat >experimentInfo.hpp <<EOF
#ifndef __WEATHERMONITOR_EXPERIMENT_INFO_HPP__
#define __WEATHERMONITOR_EXPERIMENT_INFO_HPP__

// autogenerated, don't edit!

#define GUARDED_WEATHERMONITOR 0

#if !GUARDED_WEATHERMONITOR // without vptr guards

// main() address:
// nm -C vanilla.elf|fgrep main
#define WEATHER_FUNC_MAIN			0x`addrof $1 main`
// Temperature::measure() address:
// nm -C vanilla.elf|fgrep 'Temperature::measure()'
#define WEATHER_FUNC_TEMP_MEASURE	0x`addrof $1 'Temperature::measure()'`
// number of instructions we want to observe
// 20k suffices for 4 measure() calls; we can do more later (without really learning more?)
#define WEATHER_NUMINSTR			20000
// data/BSS begin:
// nm -C vanilla.elf|fgrep ___DATA_START__
#define WEATHER_DATA_START			0x`addrof $1 ___DATA_START__`
// data/BSS end:
// nm -C vanilla.elf|fgrep ___BSS_END__
#define WEATHER_DATA_END			0x`addrof $1 ___BSS_END__`
// text begin:
// nm -C vanilla.elf|fgrep ___TEXT_START__
#define WEATHER_TEXT_START			0x`addrof $1 ___TEXT_START__`
// text end:
// nm -C vanilla.elf|fgrep ___TEXT_END__
#define WEATHER_TEXT_END			0x`addrof $1 ___TEXT_END__`

#else // with guards

// XXX

#endif

#endif
EOF

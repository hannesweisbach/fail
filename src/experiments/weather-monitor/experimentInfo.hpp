#ifndef __WEATHERMONITOR_EXPERIMENT_INFO_HPP__
#define __WEATHERMONITOR_EXPERIMENT_INFO_HPP__

// autogenerated, don't edit!

// 0 = vanilla, 1 = guarded, 2 = plausibility
#define WEATHERMONITOR_VARIANT 0

#if WEATHERMONITOR_VARIANT == 0 // without vptr guards

// suffix for simulator state, trace file
#define WEATHER_SUFFIX				".vanilla"
// main() address:
// nm -C vanilla.elf|fgrep main
#define WEATHER_FUNC_MAIN			0x00100f70
// wait_begin address
#define WEATHER_FUNC_WAIT_BEGIN		0x00100f50
// wait_end address
#define WEATHER_FUNC_WAIT_END		0x00100f60
// vptr_panic address (only exists in guarded variant)
#define WEATHER_FUNC_VPTR_PANIC		0x99999999
// number of main loop iterations to trace
// (determines trace length and therefore fault-space width)
#define WEATHER_NUMITER_TRACING		4
// number of instructions needed for these iterations in golden run (taken from
// experiment step #2)
#define WEATHER_NUMINSTR_TRACING	20549
// number of additional loop iterations for FI experiments (to see whether
// everything continues working fine)
#define WEATHER_NUMITER_AFTER		2
// number of instructions needed for these iterations in golden run (taken from
// experiment step #2)
#define WEATHER_NUMINSTR_AFTER		10232
// data/BSS begin:
// nm -C vanilla.elf|fgrep ___DATA_START__
#define WEATHER_DATA_START			0x00101814
// data/BSS end:
// nm -C vanilla.elf|fgrep ___BSS_END__
#define WEATHER_DATA_END			0x00103108
// text begin:
// nm -C vanilla.elf|fgrep ___TEXT_START__
#define WEATHER_TEXT_START			0x00100000
// text end:
// nm -C vanilla.elf|fgrep ___TEXT_END__
#define WEATHER_TEXT_END			0x0010165b

#elif WEATHERMONITOR_VARIANT == 1 // with guards

// suffix for simulator state, trace file
#define WEATHER_SUFFIX				".guarded"
// main() address:
// nm -C guarded.elf|fgrep main
#define WEATHER_FUNC_MAIN			0x00100fc0
// wait_begin address
#define WEATHER_FUNC_WAIT_BEGIN		0x00100fa0
// wait_end address
#define WEATHER_FUNC_WAIT_END		0x00100fb0
// vptr_panic address (only exists in guarded variant)
#define WEATHER_FUNC_VPTR_PANIC		0x00101460
// number of main loop iterations to trace
// (determines trace length and therefore fault-space width)
#define WEATHER_NUMITER_TRACING		4
// number of instructions needed for these iterations in golden run (taken from
// experiment step #2)
#define WEATHER_NUMINSTR_TRACING	20549
// number of additional loop iterations for FI experiments (to see whether
// everything continues working fine)
#define WEATHER_NUMITER_AFTER		2
// number of instructions needed for these iterations in golden run (taken from
// experiment step #2)
#define WEATHER_NUMINSTR_AFTER		10232
// data/BSS begin:
// nm -C guarded.elf|fgrep ___DATA_START__
#define WEATHER_DATA_START			0x00101b94
// data/BSS end:
// nm -C guarded.elf|fgrep ___BSS_END__
#define WEATHER_DATA_END			0x001034b8
// text begin:
// nm -C guarded.elf|fgrep ___TEXT_START__
#define WEATHER_TEXT_START			0x00100000
// text end:
// nm -C guarded.elf|fgrep ___TEXT_END__
#define WEATHER_TEXT_END			0x001019db

#elif WEATHERMONITOR_VARIANT == 2 // with guards + plausibility check

// suffix for simulator state, trace file
#define WEATHER_SUFFIX				".plausibility"
// main() address:
// nm -C plausibility.elf|fgrep main
#define WEATHER_FUNC_MAIN			0x00100fd0
// wait_begin address
#define WEATHER_FUNC_WAIT_BEGIN		0x00100fb0
// wait_end address
#define WEATHER_FUNC_WAIT_END		0x00100fc0
// vptr_panic address (only exists in guarded variant)
#define WEATHER_FUNC_VPTR_PANIC		0x00101500
// number of main loop iterations to trace
// (determines trace length and therefore fault-space width)
#define WEATHER_NUMITER_TRACING		4
// number of instructions needed for these iterations in golden run (taken from
// experiment step #2)
#define WEATHER_NUMINSTR_TRACING	20549
// number of additional loop iterations for FI experiments (to see whether
// everything continues working fine)
#define WEATHER_NUMITER_AFTER		2
// number of instructions needed for these iterations in golden run (taken from
// experiment step #2)
#define WEATHER_NUMINSTR_AFTER		10232
// data/BSS begin:
// nm -C plausibility.elf|fgrep ___DATA_START__
#define WEATHER_DATA_START			0x00101c94
// data/BSS end:
// nm -C plausibility.elf|fgrep ___BSS_END__
#define WEATHER_DATA_END			0x001035b8
// text begin:
// nm -C plausibility.elf|fgrep ___TEXT_START__
#define WEATHER_TEXT_START			0x00100000
// text end:
// nm -C plausibility.elf|fgrep ___TEXT_END__
#define WEATHER_TEXT_END			0x00101adb

#else
#error Unknown WEATHERMONITOR_VARIANT
#endif

#endif

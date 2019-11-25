import os
import multiprocessing
import kconfiglib

ARMTC = 'arm-none-eabi-'
UARMDIR = 'uarm'
UARMLD = '{}/elf32ltsarm.h.uarmcore.x'.format(UARMDIR)

MPSTC = 'mipsel-linux-gnu-'
UMPSDIR = 'umps'
UMPSLD = '{}/umpscore.ldscript'.format(UMPSDIR)
UMPSFLAGS = [
    '-mips1', '-mabi=32', '-mno-gpopt', '-G0', '-mno-abicalls', '-fno-pic',
    '-mfp32', '-I{}/umps'.format(UMPSDIR)
]

ENV = {
    'TARGET_UARM': {
        "AS": "{}as".format(ARMTC),
        "CC": "{}gcc".format(ARMTC),
        "LINK": "{}ld".format(ARMTC),
        "ENV": os.environ,
        "CPPPATH": ['uarm', 'uarm/uarm', 'include'],
        "ASFLAGS": ['-mcpu=arm7tdmi', '-I{}/uarm'.format(UARMDIR)],
        "CCFLAGS": ['-Wall', '-O0', '-mcpu=arm7tdmi', "-DTARGET_UARM=1"],
        "LINKFLAGS": [
            '-G0',
            '-nostdlib',
            '-T{}'.format(UARMLD),
        ],
    },
    'TARGET_UMPS': {
        "AS": "{}as".format(MPSTC),
        "CC": "{}gcc".format(MPSTC),
        "LINK": "{}ld".format(MPSTC),
        "ENV": os.environ,
        "CPPPATH": ['umps', 'include'],
        "ASFLAGS": UMPSFLAGS,
        "CCFLAGS": UMPSFLAGS + ['-DTARGET_UMPS=1'],
        "LINKFLAGS": [
            '-G0',
            '-nostdlib',
            '-T{}'.format(UMPSLD),
        ],
    }
}


def get_target():
    if 'uarm' in COMMAND_LINE_TARGETS:
        return 'TARGET_UARM'
    elif 'umps' in COMMAND_LINE_TARGETS:
        return 'TARGET_UMPS'
    elif os.path.isfile('.config'):
        config = kconfiglib.Kconfig()
        config.load_config('.config')
    else:
        config = kconfiglib.standard_kconfig()
        kconfiglib.load_allconfig(config, "alldef.config")
        config.write_config()

    for symbol in config.unique_choices:
        if symbol.name == 'target':
            return symbol.selection.name


ELF = 'kernel'
TOOLCHAIN = 'arm-none-eabi-'
CFLAGS = ['-Wall', '-O0', '-mcpu=arm7tdmi', "-D{}=1".format(get_target())]
ASFLAGS = ['-mcpu=arm7tdmi', '-I{}/uarm'.format(UARMDIR)]

Help("""
Type:\t'scons' to build the example
     \t'scons uarm' to build for the uARM emulator (configuration will be ignored)
     \t'scons umps' to build for the uMPS emulator (configuration will be ignored)
     \t'scons -c' to clean the corresponding target
""",
     append=False)

num_cpu = multiprocessing.cpu_count()
SetOption('num_jobs', num_cpu)
print("Running with -j {}".format(GetOption('num_jobs')))

target = get_target()
env_options = ENV[target]

sources = Glob("*.c")
if target == 'TARGET_UARM':
    sources += Glob("{}/*.s".format(
        UARMDIR))
elif target == 'TARGET_UMPS':
    sources += Glob("{}/*.S".format(
        UMPSDIR))

env = Environment(**env_options)

e = env.Program(ELF, sources)
env.Alias('uarm', e)

if target == 'TARGET_UMPS':
    c = env.Command('kernel.core.umps', ELF, 'umps2-elf2umps -k {}'.format(ELF))
    Clean(c, "kernel.stab.umps")
    env.Alias('umps', c)

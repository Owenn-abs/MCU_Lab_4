"""Build the existing Cube HAL project with Arm GNU tools on PATH."""
from pathlib import Path
import argparse
import shutil
import subprocess

ROOT = Path(__file__).resolve().parent

def run(args):
    subprocess.run([str(arg) for arg in args], cwd=ROOT, check=True)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--scope', action='store_true', help='Enable PB14/PB15 timing markers')
    args = parser.parse_args()
    gcc = shutil.which('arm-none-eabi-gcc')
    if not gcc:
        raise SystemExit('Add the GNU Arm Embedded Toolchain bin directory to PATH.')
    tool_dir = Path(gcc).parent
    suffix = '.exe' if Path(gcc).suffix == '.exe' else ''
    out = ROOT / ('build-scope' if args.scope else 'build')
    out.mkdir(exist_ok=True)
    flags = ['-mcpu=cortex-m3', '-mthumb', '-mfloat-abi=soft', '-std=c99',
             '-Os', '-g3', '-Wall', '-Wextra', '-ffunction-sections', '-fdata-sections',
             '-DUSE_HAL_DRIVER', '-DSTM32F103x6', f'-DLAB_SCOPE_PINS={int(args.scope)}']
    includes = ['Core/Inc', 'Drivers/STM32F1xx_HAL_Driver/Inc',
                'Drivers/STM32F1xx_HAL_Driver/Inc/Legacy',
                'Drivers/CMSIS/Include', 'Drivers/CMSIS/Device/ST/STM32F1xx/Include']
    flags += [f'-I{path}' for path in includes]
    sources = sorted((ROOT/'Core/Src').glob('*.c'))
    sources += sorted((ROOT/'Drivers/STM32F1xx_HAL_Driver/Src').glob('*.c'))
    sources += [ROOT/'Core/Startup/startup_stm32f103c6ux.s']
    objects = []
    for source in sources:
        obj = out / (source.stem + '.o')
        run([gcc, *flags, '-c', source, '-o', obj])
        objects.append(obj)
    elf = out/'Lab_4.elf'
    run([gcc, '-mcpu=cortex-m3', '-mthumb', '-mfloat-abi=soft',
         '-TSTM32F103C6UX_FLASH.ld', '--specs=nano.specs', '--specs=nosys.specs',
         '-Wl,--gc-sections', f'-Wl,-Map={out / "Lab_4.map"}',
         '-Wl,--print-memory-usage', *objects, '-o', elf])
    run([tool_dir/f'arm-none-eabi-objcopy{suffix}', '-O', 'ihex', elf, out/'Lab_4.hex'])
    run([tool_dir/f'arm-none-eabi-size{suffix}', elf])
    print(f'Built {elf} and {out / "Lab_4.hex"}')

if __name__ == '__main__':
    main()

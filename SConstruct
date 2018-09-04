TOOL_PATH='/home/kevinchen/SAMA5D3/gcc-linaro-arm-linux-gnueabihf-4.7-2013.04-20130415_linux/bin/arm-linux-gnueabihf-gcc'
#TOOL_PATH='/home/kevinchen/MYD-SAM9X25/arm-2010q1/bin/arm-none-linux-gnueabi-gcc'

arch = ARGUMENTS.get('arch', 'x86')
env = Environment(LINKFLAGS='-lpthread -pthread')
Export('env')
if arch == 'arm':
    env.Replace(CC=TOOL_PATH)

libeth_test = env.SConscript('eth_test/SConscript')
libuart_test = env.SConscript('uart_test/SConscript')
libstorage_test = env.SConscript('storage_test/SConscript')
libled_test = env.SConscript('led_test/SConscript')
env.Install('obj', [libeth_test, libuart_test, libstorage_test, libled_test])

bin = env.Program(target='odm_tap_factory_test', LIBS=['uart_test', 'eth_test', 'storage_test', 'led_test'], source = ['odm-tap-factory.c'], LIBPATH='./obj')
env.Install('out', bin)

# MYiR test project

## Compile

    make OPTION=<configure>

    OPTION:
    sama5d3x_factory 	-- sama5d3x factory test
    zhiyi_factory 		-- zhiyi factory test
    am335x_factory 		-- am335x factory test
    am335x_all			-- test all the periphery
    sam9x5v1_all		-- sam9x5v1 factor test


## Ouput
Binary file will be put in "./out" directory.

## Usage

run the execute file on the board, you will get test menu list.This program support two modes, manual and auto.
If you run give an parameter, this program will auto test all items of your pre-defined.

    ./binary-name -a

### USB Testing

The AT91SAM9X5 series board has two USB interfaces, USB host and USB mini OTG.When you prepare to test these modules,
you must use an USB convert cable(One side is USB TypeA plug, other side is USB MiniB plug).
And you must install a kernel module named "g_mass_storage.ko".

On this test method, USB OTG as device mode.It will communicate with USB Host.

Caution, this method can not test OTG as Host mode.

## MYS-SAM9X25

when test prgram is running, the D1 led will follow heartbeat blink.While the test result will failed, the blue led will off,
red led will turn on always.If testing is success, the blue led will turn on always, not blink with heartbeat.

## ODM TAP Factory Test
The TAP project is used with scons to build test program.
    scons arch=arm

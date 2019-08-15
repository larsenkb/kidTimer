SKETCH = kidTimer.ino

BOARD = teensyLC
#BOARD = teensy31
#BOARD = genericSTM32F103C
#BOARD = BluePill

ifeq ($(BOARD),genericSTM32F103C)
	PACKAGE = Arduino_STM32
	ARCH    = STM32F1
	BOARD   = genericSTM32F103C
	UPLOAD_METHOD = upload_method=DFUUploadMethod
	PARAMS  = device_variant=STM32F103CB,${UPLOAD_METHOD},cpu_speed=speed_72mhz,opt=osstd
	BOARD_LIBS = ~/Arduino/hardware/$(PACKAGE)/$(ARCH)/libraries
	BOARD_CORES = ~/Arduino/hardware/$(PACKAGE)/$(ARCH)/cores
else ifeq ($(BOARD),BluePill)
	PACKAGE = STM32GENERIC
	ARCH    = STM32
	BOARD   = BluePill
	UPLOAD_METHOD = upload_method=DFUUploadMethod
	PARAMS  = ${UPLOAD_METHOD},usb=SerialUSB,serial=Automatic,jtag_swd=Disabled 
	BOARD_LIBS = ~/Arduino/hardware/$(PACKAGE)/$(ARCH)/libraries
	BOARD_CORES = ~/Arduino/hardware/$(PACKAGE)/$(ARCH)/cores
else ifeq ($(BOARD),teensyLC)
	PACKAGE = teensy
	ARCH = avr
	BOARD = teensyLC
	PARAMS = usb=serial,speed=48,opt=osstd,keys=en-us
	BOARD_CORES = /usr/share/arduino/hardware/teensy/avr/cores/teensy3
	BOARD_LIBS = /usr/share/arduino/hardware/teensy/avr/libraries
else ifeq ($(BOARD),teensy31)
	PACKAGE = teensy
	ARCH = avr
	BOARD = teensy31
	PARAMS = usb=serial,speed=96,opt=o2std,keys=en-us
	BOARD_CORES = /usr/share/arduino/hardware/teensy/avr/cores/teensy3
	BOARD_LIBS = /usr/share/arduino/hardware/teensy/avr/libraries
endif

BUILDPATH = build
BUILDPREF = --pref build.path=$(BUILDPATH)

#VERBOSE = --verbose
VERBOSE = 

PORTDEV = /dev/ttyACM0
PORT = --port $(PORTDEV)

.PHONY: ctags upload verify clean

verify:
	arduino $(VERBOSE) --verify --board ${PACKAGE}:${ARCH}:${BOARD}:${PARAMS} ${BUILDPREF}  $(PORT) $(SKETCH) 
	#arduino $(VERBOSE) --verify --board ${BOARDBUILD} ${BUILDPREF}  $(PORT) $(SKETCH) 

upload:
	arduino $(VERBOSE) --upload --board ${PACKAGE}:${ARCH}:${BOARD}:${PARAMS} ${BUILDPREF} $(PORT) $(SKETCH)

ctags:
	ctags -R --langmap=c++:+.ino 
	ctags -aR ${BOARD_CORES}
	ctags -aR ${BOARD_LIBS}
	ctags -aR /usr/share/arduino/libraries
	ctags -aR ~/Arduino/libraries

clean:
	rm -rf ${BUILDPATH}
	rm -f tags

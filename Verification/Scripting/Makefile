PLATFORM=$(shell uname)
ifeq ($(PLATFORM),MINGW64_NT-10.0)
  PLAT=mingw
  BUILD_TARGET=mingw_win
else
  PLAT=gnu_linux
endif
SMV=../../Build/smokeview/$(BUILD_TARGET)_64/smokeview_${PLAT}_64${EXT}
SMVOPTS=-killscript

default:
	echo $(PLATFORM)
	echo "MINGW64_NT-10.0"
	echo $(PLAT)
	# this tests from a different path: this currently doesn't work
	# $(SMV) $(SMVOPTS) -luascript tests/test1.lua room_fire/room_fire > test1.log
	echo
	# this tests from within the directory
	cd room_fire && pwd && ../$(SMV) $(SMVOPTS) -luascript ../tests/test1.lua room_fire > test1a.log

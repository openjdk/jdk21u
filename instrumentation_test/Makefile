PLATFORM := $(shell uname -s)

FILE := Carv.java

ifeq ($(PLATFORM),Darwin)
  BUILD_FOLDER := build/macosx-aarch64-server-release
else
  BUILD_FOLDER := build/linux-x86_64-server-release
endif

ifeq ($(PLATFORM),Darwin)
  DEBUG_BUILD_FOLDER := build/macosx-aarch64-server-fastdebug
else
  DEBUG_BUILD_FOLDER := build/linux-x86_64-server-fastdebug
endif

JAVA_BIN := ../$(BUILD_FOLDER)/jdk/bin
JAVA_DEBUG_BIN := ../$(DEBUG_BUILD_FOLDER)/jdk/bin

g1:
	$(JAVA_BIN)/javac $(FILE)
	$(JAVA_BIN)/java -XX:+PrintKlassInfo $(basename $(FILE))
	rm -f $(basename $(FILE)).class

zgc:
	$(JAVA_BIN)/javac $(FILE)
	$(JAVA_BIN)/java -XX:+UseZGC -XX:+ZGenerational -XX:+PrintKlassInfo $(basename $(FILE))


gdb:
	$(JAVA_DEBUG_BIN)/javac $(FILE)
	gdb --args $(JAVA_DEBUG_BIN)/java -XX:+UseZGC -XX:+ZGenerational -XX:+PrintKlassInfo $(basename $(FILE))

update:
	(cd .. && git pull && make CONF=linux-x86_64-server-release)

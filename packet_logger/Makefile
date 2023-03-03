BUILD_DIRECTORY = build
SOURCES_DIRECTORY = sources

STANDARD = -std=gnu17

LIBRARY = -lpcap

all:
	mkdir -p $(BUILD_DIRECTORY)
	gcc -Wall ${STANDARD} $(SOURCES_DIRECTORY)/*.c -o $(BUILD_DIRECTORY)/program $(LIBRARY)
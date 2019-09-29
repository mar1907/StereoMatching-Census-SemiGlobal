all: build

build: OpenCVApplication

OpenCVApplication: OpenCVApplication.cpp
	g++ $< `pkg-config opencv --cflags --libs` -o $@

run: OpenCVApplication
	./$<

clean:
	rm -rf OpenCVApplication results/

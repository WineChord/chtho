BUILDDIR := build 


.PHONY: clean build rebuild

build:
	mkdir -p ${BUILDDIR}
	cd ${BUILDDIR} && cmake .. && make

rebuild:
	rm -rf ${BUILDDIR}
	mkdir -p ${BUILDDIR}
	cd ${BUILDDIR} && cmake .. && make

clean:
	rm -rf ${BUILDDIR}


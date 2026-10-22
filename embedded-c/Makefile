SUBDIRS = \
	ble_tracker \
	bit_manipulation \
	ring_buffers \
	parsers_drivers \
	pointers_algorithms \
	endian_practice \
	atoi_itoa_practice \
	concurrency \
	concurrency/conditionalvariables \
	device_drivers

.PHONY: all clean $(SUBDIRS)

all:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

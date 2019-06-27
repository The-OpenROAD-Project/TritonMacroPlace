SUBDIR = module/lef/5.8-p027 module/def/5.8-p027 
OPENSTADIR = module/OpenSTA
REPLACEDIR = src


all: prep sta 
	$(MAKE) -C $(REPLACEDIR);

prep: 
	for dir in $(SUBDIR); do \
		$(MAKE) -C $$dir; \
	done

sta: 
	cd $(OPENSTADIR) && mkdir -p install-sp && \
		libtoolize && ./bootstrap && \
		./configure --prefix=$(CURDIR)/$(OPENSTADIR)/install-sp && \
		$(MAKE) && \
		$(MAKE)	install;

clean:
	for dir in $(SUBDIR); do \
		$(MAKE) -C $$dir clean; \
	done;
	cd $(OPENSTADIR) && $(MAKE) distclean && rm -rf install-sp > /dev/null 2>&1; true
	$(MAKE) -C $(REPLACEDIR) clean;

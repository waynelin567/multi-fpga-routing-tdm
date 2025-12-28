STATIC ?= false
TSAN  ?= false
ASAN  ?= false

ECHO   = /bin/echo
EXEC   = router
DIRS   = bin

all: directory

directory:
	@for dir in $(DIRS); \
	do \
		if [ ! -d $$dir ]; then \
			mkdir $$dir; \
			echo "Creating directory \"$$dir\" ..."; \
		fi; \
	done
	@$(MAKE) -C src -f Makefile.src --no-print-directory EXEC=$(EXEC) STATIC=$(STATIC) TSAN=$(TSAN) ASAN=$(ASAN);
	@ln -fs bin/$(EXEC)

clean:
	@$(MAKE) -C src -f Makefile.src --no-print-directory clean;
	@rm -f $(EXEC)

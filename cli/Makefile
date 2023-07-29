# C source files
CODEDIRS=./src ./src/utils
# Headers
INCDIRS=./src/include
# Output directory for the main executables
OUTDIR=./build/

# C compiler
CC=gcc
#ARCH=-m32
# Optimization
OPT=-O0
# Generate files that encode make rules for the .h dependencies
DEPFLAGS=-MP -MD
# Automatically add the -I onto each include directory
CFLAGS=-Wall -Wextra $(ARCH) -g $(foreach D,$(INCDIRS),-I$(D)) $(OPT) $(DEPFLAGS)
# Link SQLite
LINK=-lsqlite3

# For-style iteration (foreach) and regular expression completions (wildcard)
CFILES=$(foreach D,$(CODEDIRS),$(wildcard $(D)/*.c))
# Regular expression replacement
OBJECTS=$(patsubst %.c,%.o,$(CFILES))
DEPFILES=$(patsubst %.c,%.d,$(CFILES))

all: binaries

install: all
	@echo "Installing the main binary to /bin"
	@install -v $(OUTDIR)gawake-cli /usr/bin/	# src destination
	@echo "Installing the helper binaries to /opt/gawake/bin/cli"
	@install -v -o root -g root -m 754 -D -t /opt/gawake/bin/cli/ $(OUTDIR){db_checker,cron_toff,cron_ton} # destination src
	@echo "Installing the cron service to /etc/cron.d"
	@# Uninstaller
	@install -v -o root -g root -m 555 uninstall.sh /opt/gawake/	# src destination
	@install -v -o root -g root -m 644 gawake /etc/cron.d/	# src destination
	@echo "Adding Gawake user"
	-@useradd -M gawake
	@echo "Gawake installed successfully!"

binaries: $(OBJECTS)
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) $(LINK) src/cron_toff.o src/utils/get_time.o src/utils/wday.o -o $(OUTDIR)cron_toff
	$(CC) $(CFLAGS) $(LINK) src/cron_ton.o src/utils/get_time.o -o $(OUTDIR)cron_ton
	$(CC) $(CFLAGS) $(LINK) src/db_checker.o src/utils/issue.o -o $(OUTDIR)db_checker
	$(CC) $(CFLAGS) $(LINK) src/main.o src/utils/get_time.o src/utils/wday.o src/utils/issue.o -o $(OUTDIR)gawake-cli

# Only want the .c file dependency here, thus $< instead of $^.
%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm -rf $(OBJECTS) $(DEPFILES) $(OUTDIR)

# Include the dependencies
-include $(DEPFILES)

# Add .PHONY so that the non-targetfile - rules work even if a file with the same name exists.
.PHONY: all clean

# REFERENCES:
# Install:	https://www.man7.org/linux/man-pages/man1/install.1.html 	;	https://www.baeldung.com/linux/install-command
# Makefile:	https://www.gnu.org/software/make/manual/html_node/index.html 	;	https://www.youtube.com/watch?v=DtGrdB8wQ_8


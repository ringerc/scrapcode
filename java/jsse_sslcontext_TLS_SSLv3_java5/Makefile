all: SSLContextInfo JSSInfo

%Info: %Info.class
	@echo
	java $(basename $< .class)

%.class: %.java
	javac $<

.PHONY: compiledb

compiledb:
	mkdir -p $(COMPILEDB_ROOT)/build/cmake \
	&& cd $(COMPILEDB_ROOT)/build/cmake \
    && cmake $(COMPILEDB_ROOT) -BDebug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=YES $(COMPILEDB_CMAKE_ARGS)
	bash -c 'if [ ! -f $(COMPILEDB_ROOT)/compile_commands.json ]; then \
		ln -s build/cmake/Debug/compile_commands.json $(COMPILEDB_ROOT)/compile_commands.json; \
	fi'

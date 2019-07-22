.PHONY: compiledb

compiledb:
	mkdir -p $(COMPILEDB_ROOT)/$(if $(COMPILEDB_OUTPUT),$(COMPILEDB_OUTPUT),build/cmake) \
	&& cd $(COMPILEDB_ROOT)/$(if $(COMPILEDB_OUTPUT),$(COMPILEDB_OUTPUT),build/cmake) \
    && cmake $(COMPILEDB_ROOT) -Wno-dev -BDebug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=YES $(COMPILEDB_CMAKE_ARGS) \
	&& rm -f $(COMPILEDB_ROOT)/compile_commands.json \
	&& ln -s $(if $(COMPILEDB_OUTPUT),$(COMPILEDB_OUTPUT),build/cmake)/Debug/compile_commands.json $(COMPILEDB_ROOT)/compile_commands.json

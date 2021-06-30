.PHONY: compiledb

compiledb:
	$(if $(CMAKE),$(CMAKE),cmake) $(COMPILEDB_ROOT) \
		-B$(COMPILEDB_ROOT)/$(if $(COMPILEDB_OUTPUT),$(COMPILEDB_OUTPUT),build/cmake) \
		-Wno-dev -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=YES $(COMPILEDB_CMAKE_ARGS) \
	&& rm -f $(COMPILEDB_ROOT)/compile_commands.json \
	&& ln -s $(if $(COMPILEDB_OUTPUT),$(COMPILEDB_OUTPUT),build/cmake)/compile_commands.json $(COMPILEDB_ROOT)/compile_commands.json

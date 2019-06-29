.PHONY: compiledb

compiledb: compiledb-$(COMPILEDB_XCODE_SCHEME)

compiledb-$(COMPILEDB_XCODE_SCHEME): clean-$(COMPILEDB_XCODE_SCHEME)
	xcodebuild -workspace $(XCODE_WORKSPACE) -scheme $(COMPILEDB_XCODE_SCHEME) -configuration $(XCODE_CONF) build \
		| xcpretty --no-color -r json-compilation-database --output $(COMPILEDB_ROOT)/compile_commands.json

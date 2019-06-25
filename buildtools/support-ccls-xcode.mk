.PHONY: ccls
ccls: ccls-$(CCLS_SCHEMA)
ccls-$(CCLS_SCHEMA): clean-$(CCLS_SCHEMA)
	xcodebuild -workspace $(XCODE_WORKSPACE) -scheme $(CCLS_SCHEMA) -configuration $(XCODE_CONF) build \
		| xcpretty --no-color -r json-compilation-database --output $(CCLS_ROOT)/compile_commands.json

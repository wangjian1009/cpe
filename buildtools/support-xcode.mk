.PHONY: build install clean

build:

define xcode-define-scheme

.PHONY: build-$1 install-$1 clean-$1

build: build-$1
install: install-$1
clean: clean-$1

.PHONY: $1
$1: build-$1

build-$1:
	xcodebuild -workspace $(XCODE_WORKSPACE) -scheme $1 -configuration $(XCODE_CONF) build \
		| xcpretty --no-utf

install-$1:
	xcodebuild -workspace $(XCODE_WORKSPACE) -scheme $1 -configuration $(XCODE_CONF) install \
		| xcpretty --no-utf

clean-$1:
	xcodebuild -workspace $(XCODE_WORKSPACE) -scheme $1 clean \
		| xcpretty --no-utf

endef

$(eval $(foreach s,$(XCODE_SCHEMES),$(call xcode-define-scheme,$s)))

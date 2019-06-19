.PHONY: build install clean

build:
	xcodebuild -workspace $(XCODE_WORKSPACE) -scheme $(XCODE_SCHEME) -configuration $(XCODE_CONF) build | xcpretty --no-color

install:
	xcodebuild -workspace $(XCODE_WORKSPACE) -scheme $(XCODE_SCHEME) -configuration $(XCODE_CONF) install | xcpretty --no-color

clean:
	xcodebuild -workspace $(XCODE_WORKSPACE) -scheme $(XCODE_SCHEME) clean | xcpretty

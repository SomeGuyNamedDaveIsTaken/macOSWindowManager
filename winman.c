#include <ApplicationServices/ApplicationServices.h>

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

static CFMachPortRef g_tap = NULL;

static void await_accessiblity_permissions(void) {
    const void *keys[] = {kAXTrustedCheckOptionPrompt};
    const void *values[] = {kCFBooleanTrue};
    CFDictionaryRef options = CFDictionaryCreate(
        NULL,
        keys,
        values,
        1,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks
    );

    bool trusted = AXIsProcessTrustedWithOptions(options);
    CFRelease(options);

    bool loggedError = false;
    while (!trusted) {
        if (!loggedError) {
            fprintf(stderr, "Process not trusted. Awaiting permission to be granted in settings.\n");
            loggedError = true;
        }
        sleep(2);
        trusted = AXIsProcessTrusted();
    }
}

static CGEventRef tapEventCallback(CGEventTapProxy tapProxy, CGEventType eventType, CGEventRef event, void *refcon) {
    // If disabled event, re-enable tap.
    if (eventType == kCGEventTapDisabledByTimeout || eventType == kCGEventTapDisabledByUserInput) {
        CGEventTapEnable(g_tap, true);
        return event;
    }
    // Only fire callback on keydown events.
    if (eventType != kCGEventKeyDown) {
        return event;
    }
    // Ignore autorepeating key presses.
    if (CGEventGetIntegerValueField(event, kCGKeyboardEventAutorepeat)) {
        return event;
    }
    // TODO: Check if key press exists in bindings.
    // TODO: Get bundle ID from key press.
    // TODO: Check that bundle ID exists.
    // TODO: Call helper function to open or focus the bundle ID and return NULL (swallow key press).
    return event;
}

int main(void) {
    fprintf(stdout, "Waiting for accessibility permission...\n");
    await_accessiblity_permissions();
    fprintf(stdout, "...accessibility permission granted.\n");

    // TODO: Read in config and create lookup data structure to go from e.g. control+shift+v to app bundle ID.

    g_tap = CGEventTapCreate(
        kCGSessionEventTap,
        kCGHeadInsertEventTap,
        kCGEventTapOptionDefault,
        CGEventMaskBit(kCGEventKeyDown),
        tapEventCallback,
        NULL
    );
    if (!g_tap) {
        fprintf(stderr, "Tap creation failed.\n");
        return 1;
    }

    CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, g_tap, 0);
    if (source == NULL) {
        fprintf(stderr, "Unable to create run loop source.\n");
        CFRelease(g_tap);
        g_tap = NULL;
        return 1;
    }
    CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
    CFRelease(source);
    CGEventTapEnable(g_tap, true);

    CFRunLoopRun();

    return 0;
}

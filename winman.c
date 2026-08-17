#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include <ApplicationServices/ApplicationServices.h>

#define MAX_BUNDLE 128
#define MAX_BINDINGS 64
#define POLLING_TIMEOUT_SECONDS 2

typedef struct {
    CGEventFlags mods;
    CGKeyCode key;
    char bundle[MAX_BUNDLE];
} Binding;

static CFMachPortRef g_tap = NULL;
static Binding g_bindings[MAX_BINDINGS];
static size_t g_bindings_size = 0;

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
        sleep(POLLING_TIMEOUT_SECONDS);
        trusted = AXIsProcessTrusted();
    }
}

int load_config(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "File not found at %s\n", path);
        return -1;
    }

    char line[256];
    int line_number = 0;
    g_bindings_size = 0;

    while (fgets(line, sizeof(line), file)) {
        line_number++;

        if (!strchr(line, '\n') && !feof(file)) {
            fprintf(stderr, "config line %d: too long, skipping\n", line_number);
            int c;
            while ((c = fgetc(file)) != EOF && c != '\n')
                ;
            continue;
        }

        char *line_pointer = line;
        while (*line_pointer == ' ' || *line_pointer == '\t') {
            line_pointer++; // skip leading space
        }
        if (*line_pointer == '\0' || *line_pointer == '\n' || *line_pointer == '#') {
            continue; // blank / comment
        }

        char chord[64], bundle[MAX_BUNDLE];
        if (sscanf(line_pointer, "%63s %127s", chord, bundle) != 2) { // widths cap overflow
            fprintf(stderr, "config line %d: malformed, skipping\n", line_number);
            continue;
        }
        if (g_bindings_size >= MAX_BINDINGS) {
            fprintf(stderr, "config: more than %d bindings, ignoring rest\n", MAX_BINDINGS);
            break;
        }

        CGEventFlags mods;
        CGKeyCode key;
        // if (parse_chord(chord, &mods, &key) != 0) {
        //     fprintf(stderr, "config line %d: bad chord '%s', skipping\n", lineno, chord);
        //     continue;
        // }

        Binding *b = &g_bindings[g_bindings_size++];
        b->mods = mods;
        b->key = key;
        snprintf(b->bundle, sizeof(b->bundle), "%s", bundle);
    }

    fclose(file);
    return (int)g_bindings_size;
}

static CGEventRef tap_event_callback(CGEventTapProxy tapProxy, CGEventType eventType, CGEventRef event, void *refcon) {
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
        tap_event_callback,
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

# RadoIDE

A minimal C IDE for Windows built with GTK3 and GtkSourceView, created as a one-month school project.

## Features

- **Syntax Highlighting:** GtkSourceView-powered C language support with current-line highlighting
- **Smart Editor:** Line numbers, auto-indent, and bracket completion for `(`, `[`, and `{`
- **File Tree:** Recursive project directory view with create, open, and delete support
- **Find:** Real-time match highlighting with next/previous navigation and wrap-around
- **Build & Run:** Compile and execute C code directly from the IDE via an embedded MSYS terminal
- **Motivation:** Revolutionary productivity enhancement feature :D

## Requirements

- Windows (RadoIDE uses Windows-specific APIs and targets MSYS2/MinGW64)
- [MSYS2](https://www.msys2.org/) with MinGW64

## Build

### Install dependencies

From the MinGW64 terminal:

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gtk3 mingw-w64-x86_64-gtksourceview3 make
```

### Clone and build

```bash
git clone https://github.com/braniik/radoide-1.0.git
cd radoide-1.0
make
```

The binary is placed in `build/radoide.exe`. You can also use `make run` to build and launch in one step.

> **Note:** Make sure you're using the **MinGW 64-bit terminal**, not MSYS or UCRT64. If you see `Package gtk+-3.0 was not found`, the dev packages above aren't installed.

### Makefile targets

| Target | Description |
|--------|-------------|
| `make` | Build the project (default) |
| `make run` | Build and run |
| `make clean` | Remove `build/` |

## Project structure

```
radoide-1.0/
├── makefile                   — build config
├── radoide.gresource.xml      — GLib resource bundle (logo)
├── include/
│   ├── main.h                 — app-wide structs and GTK widget references
│   ├── editor.h               — bracket indentation
│   ├── file_tree.h            — file tree declarations
│   ├── file_operations.h      — open, save, create, delete
│   ├── find_replace.h         — search state and callbacks
│   ├── build_system.h         — compile/run logic
│   ├── terminal.h             — terminal panel
│   ├── styling.h              — CSS helpers
│   └── motivation.h           — critical feature
└── src/
    ├── main.c                 — app startup, UI assembly, signal wiring
    ├── editor.c               — bracket indentation on keypress
    ├── file_tree.c            — tree population and interaction
    ├── file_operations.c      — file I/O and tree refresh
    ├── find_replace.c         — incremental search, match navigation
    ├── build_system.c         — gcc invocation, terminal toggle
    ├── terminal.c             — embedded MSYS terminal panel
    ├── styling.c              — GTK CSS loading helpers
    └── motivation.c           — critical feature implementation
```

## Architecture

Startup runs through `main` → `on_activate`, which builds the full UI in one pass: main window, menu bar, file tree, editor, terminal panel, CSS, and all signal connections. After that the app is entirely event-driven, `on_activate` hands off control to GTK's main loop and every subsequent action goes through module callbacks.

The editor is a `GtkSourceView` configured with C language support. Bracket completion is handled by a `key-press-event` signal that intercepts `(`, `[`, and `{` to insert the closing character and position the cursor.

The file tree is rebuilt from disk after every create/delete operation to stay in sync with the real filesystem. Clicking a directory sets the working directory for the build system; clicking a file loads it into the editor.

The build system module shells out to `gcc` through the embedded terminal. Since the project targets Windows, MSYS2 provides the Unix-like environment needed to run standard tooling.

The find bar does incremental text buffer scanning on every keystroke, tagging all matches. A position cursor tracks the last jump so next/previous knows where to continue from, wrapping at file boundaries. Closing the bar clears all tags and resets state.

## License

MIT — fork, modify, and distribute freely.


### Requirements

- **GCC** (GNU C Compiler)
- **GTK+ 3.0** (GUI toolkit)
- **GtkSourceView 3.0** (Source code editing widget with syntax highlighting)
- **GLib** (comes with GTK for resource compilation)
- **Make** (Build automation)
- **pkg-config** (For managing library compilation flags)
- **Windows Only** (RadoIDE uses Windows-specific APIs and is designed to run on Windows. Build using MSYS2/MinGW64.)

### Installing Dependencies

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gtk3 mingw-w64-x86_64-gtksourceview3 make
```

### Build Instructions

1. Clone the repository:
```bash
   git clone https://github.com/braniik/radoide-1.0.git
   cd RadoIDE
```

2. Build the project:
```bash
   make
```

3. Run RadoIDE

### Troubleshooting

- If you get **"Package gtk+-3.0 was not found"** errors, install the development packages listed above.

-  **Make sure you're using the MinGW 64-bit terminal**

### About
Created by me as a half-year school project in the span of +- a month.

### License
MIT License - feel free to fork, modify, and distribute!

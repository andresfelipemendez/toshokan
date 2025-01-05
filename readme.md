# Toshokan (図書館)

A minimalist PDF reader and library manager focused on simplicity and productivity. Built with SDL3 and MuPDF.

*Toshokan (図書館, としょかん) means "library" in Japanese, reflecting the project's goal of organizing and managing your digital book collection.*

## Project Vision

Toshokan was born from a common problem: having so many PDF books that you lose track of what you have. It aims to solve the "digital bookshelf overflow" where PDFs accumulate across folders, making it hard to discover and remember what you've collected.

Think of it as "Sublime Text for PDFs" - not just another PDF reader, but a tool to help you:
- Rediscover books you forgot you had
- Actually find that paper you remember downloading months ago
- Keep your reading organized without complex folder structures
- Make meaningful connections between related documents

### Key Features

- **Multi-tab Support**: Open the same PDF in multiple tabs, similar to how text editors work
- **Deep-linked Annotations**: Bookmarks, highlights, and notes that reference specific locations in PDFs
- **Clean UI**: Minimalist interface focused on reading and navigation
- **Fast Navigation**: Quick access to your PDF collection
- **Library Management**: Help you organize and find your accumulated PDFs

## Technical Stack

- **SDL3**: Cross-platform windowing and input handling
- **MuPDF**: PDF rendering engine
- **Clay**: UI layout system
- **Custom Build System**: On-demand compilation with hot reload support

## Building from Source

### Windows Requirements

1. Visual Studio with C/C++ support
   - Install Visual Studio Community Edition
   - Select "Desktop development with C++" workload
   - Use "x64 Native Tools Command Prompt for VS" to build

2. Required Libraries
   - SDL3
   - MuPDF
   - Clay

### Build Instructions

```batch
# First time setup
build.exe

# Regular build
build.exe build

# Clean build
build.exe rebuild
```

## Development Philosophy

- **Minimalist Design**: Features are added thoughtfully to maintain simplicity
- **Performance First**: Fast startup and smooth rendering are priorities
- **Developer Friendly**: 
  - Hot reload support for rapid development
  - Clean, documented codebase
  - Simple build system

## Current Status

🚧 **Early Development** 🚧

## Design Choices

### Why SDL3?
- Cross-platform support
- Low-level control
- Good performance
- Active development

### Why MuPDF?
- Clean, simple API
- High-quality rendering
- Lightweight

### Why Clay?
- Simple UI layout system
- Easy to extend
- Good fit for minimalist design

### Why On-demand Compilation?
- More reliable than file watching
- Works consistently across platforms
- Simple to implement and maintain

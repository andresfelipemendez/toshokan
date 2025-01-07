# Toshokan  
means library in Japanese  
  
## Project goals
- I want to find what books do I have, I've accumulated a lot of pdfs and Sumatra UI feels bloated,
- I want to be able to open the same pdf but multiple tabs, similar to a text editor like sublime text
- Annotations:
    - bookmarks, 
    - highlights and 
    - notes that reference into the pdf, 
        - similar to a "deep link" 

## Implementation goals
cross platform support
SDL3 crossplatform windowing and input, 
Clay for easy UI layout
mupdf to render the pdfs

I'm used to work with hot reloading, I've learned that watching a dir it's not really cross platform so I'll
embed some "on demand" compilation instead, every time I call build it will compile everything

I'll start with visual studio solution since I need to use this tool and working on a build.c first has been delaying development
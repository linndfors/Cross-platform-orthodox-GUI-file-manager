# Cross-platform-orthodox-GUI-file-manager
## Our team:
- Maslenchenko Oleksandra
- Yakovkin Mykola
- Nahurna Olha

  ## Prerequisites

❯ Ubuntu or WSL or MacOS
❯ GCC
❯ Git
❯ Qt6

## Goal:
In contemporary computing, efficient file management is pivotal for user interaction with operating systems. This project presents a Cross-Platform Orthodox Graphical User Interface (GUI) File Manager developed using the Qt framework.

The goal is to offer users a seamless, intuitive, and consistent file navigation experience across major operating systems, including Windows, macOS, and various Linux distributions. Adhering to orthodox principles of simplicity and functionality, the file manager employs a dual-pane layout, customizable keyboard shortcuts, an extensible plugin architecture, and diverse file operations support.
## Available operations:
- copy
- move
- delete
- view
- archive
- new file/dir
- search
- compare
- switch mode
- sort by

## Compilation/Execution
### Ubuntu:
```shell
qmake
make
./file_manager
```
### MacOS:
```shell
qmake
make
cd ./file_manager.app/Contents/MacOS/
./file_manager
```
### Windows:
```shell
qmake
mingw-w64-make
./file_manager
```

## Resources:
https://youtube.com/playlist?list=PLS1QulWo1RIZiBcTr5urECberTITj7gjA&si=k_nxoQdJTPAKRBGi<br>
https://opensource.com/article/22/12/linux-file-manager-qtfm<br>
https://www.pcsuggest.com/best-qt-based-lightweight-file-managers-for-linux/

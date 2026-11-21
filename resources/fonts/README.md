# Default engine font

`Inter-Regular-Latin1.otf` is Inter Regular (https://github.com/rsms/inter), licensed under the
SIL Open Font License 1.1 (see `OFL.txt`). It was subset to printable ASCII and Latin-1
(U+0020–U+007E, U+00A0–U+00FF) with the kerning table kept, using:

    pyftsubset Inter-Regular.otf --unicodes="U+0020-007E,U+00A0-00FF" --layout-features=kern \
        --output-file=Inter-Regular-Latin1.otf

The file is compiled into the engine (see `cmake/EmbedFile.cmake`) and backs `Font()`, so the engine
always has a font without depending on the game's Assets folder or on fonts installed on the system.

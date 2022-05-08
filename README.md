# prism - colorize stdout

Rewrite stdin to add color by either column, word, field, paragraph or enire contents.

## Notable features

* -L : leave color escape codes in input, undisturbed
* -l : replicate lolcat (using their subroutines)
* -F[FS] : in addition to colorize bye fields, adjust columns to align

## Usage
```
May  8 2022 11:04:18

usage: prism [-:c:8aBbfF:glLmn:p:Prs:SwtTH:V:Zd:uh1] [FILE]
colorize text either by character, column, row, or field

  -c CNT: change color every 2 units
  -8: 8 bit    colors      [False]
  -b: greenbar colors
  -g: rainbow  colors
  -l: lolcat   colors
  -L: leave existing escape codes along
  -H: lolcat horizontal frequency :0.230
  -V: lolcat vertical   frequency :0.100
  -m: metal    colors
  -n CNT: max of 999 items to colorize
  -p [0-35]: alternate palettes
  -B: set background color [False]
  -f: fix contrast levels  [False]
  -P: change color by paragraph
  -r: change color by row
  -s palette_list: specify palette index for each column
  -S: strip color
  -w: change color by word
  -a: align on SEP [False]
  -F SEP : change color by field [ ]
  -t: show color palettes  [False]
  -T: show brightness val  [False]
  -d INTEGER    (0)

  -bold : set output text to bold
  -foreground FF0000 : set single color for output

A custom palette can be set using the environment variable 'PRISM'.
Ex: export PRISM="#00FF00#00E000#00C000#00A000#008000"
or  setenv PRISM "#00FF00#00E000#00C000#00A000#008000"
```

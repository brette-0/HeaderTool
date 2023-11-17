# HeaderTool
-----

HeaderTool is a lightweight terminal-operated NES ROM Headering tool used to header, trim headers and rename NES ROMs using a Github hosted database or a locally stored one. To use it, simply download the binary to the working directory of where your ROMs are located, if preferable the binary could be located at a user designated folder within the root of the C drive and added to system PATH for quick accessibility. 

*It should be noted that database downloading will always attempt to download to `./headers` in the working directory of the binary, if the user wishes to use their downloaded local copy of the database anywhere then the `./headers` path may be added to system PATH to (however this is somewhat discouraged)*

HeaderTool can be used on a 32 bit Windows 7 device or onwards.

##### The help argument

```
headertool -h             // provides help message
headertool --help         // LICENSE, Discord, Github
```

##### The get argument

```
headertool -g             // download database to ./headers/
headertool --get
headertool --get temp     // download database and store in ./temp/headers/
```

##### No key word arguments

```
headertool smb1.nes "Zelda Collection" metroid.nes "Kirby Collection"
```

##### The output argument

```
headertool -o temp "Favourite Games"
headertool -output temp "Favourite Games"   // downloads to ./temp/output/
```

##### The no rename argument

```
headertool -nr smb1.nes    // header game to ./output/ with changing name
headertool --norename smb1.nes
```

##### The no header argument

```
headertool -nh smb1.nes    // renames game in ./output/ without headering
headertool --noheader smb1.nes
```

##### The clean argument

```
headertool -c smb1.nes     // clears all headers and renames in ./output
headertool --clean smb1.nes
```

##### The verbose argument

```
headertool -v smb1.nes LICENSE // will display a message declaring the skipping of the non NES file
headertool --verbose smb1.nes LICENSE
```

##### The prefer local argument

```
headertool -l smb1.nes
headertool --local smb1.nes     // attempt to use local database first (faster)
```

##### The no skip arguement

```
headertool -ns smb1.nes ZELDA
headertool --noskip smb1.nes ZELDA     // will complete with files with no .nes extension
```

##### They can be stacked like so

```
headertool -ns -nr -c -v -l -o output smb1.nes LICENSE "favorite games" zelda.nes METROID
headertool -ns -nh -v -l -o output smb1.nes LICENSE "favourite games" zelda.nes METROID
```

`-nh` is incompatible with both `-nr` and `-c` due to them either clashing in purpose or resulting in no operation.

#### Building information

`HeaderTool (1.5) [x64]` was built with `g++` in a `mingw64` environment. Installing `libcurl`, and `jsoncpp` will provide you with all the libraries needed to build this. `HeaderTool (1.5) [x86]` was built with `g++` in a `mingw32` environment with the same external libraries.

# HeaderTool 
##### `Python Script - 1.3`

```
pip install headertool
headertool path/to/rom
```

This is the README for the Python version of HeaderTool which is designed for Unix/Posix Operating Systems. 

Untested but expected support for `python 3.7`, works with `python 3.9` and was developed with `python 3.11`. Has not been tested in a 32 bit environment. Dependancies are built-in as of `python 3.7` at most.

The script performs takes a series of file paths as arguements which are then sequentially opened into memory where they undergo some basic validation. Since the ROMs are expected to be dumped we perform some basic validation. 

Checks: 

- ROM filename ends `.nes`
- ROM filesize does not trigger bits `0x137f`
- ROM is trimmed of first 16 bits if set in filesize.

After checks pass, a checksum is calculated and used as filename to get header and goodname of ROM. If `github.usercontent` reports a 404 then invalid or unidentified ROM message is returned. Other errors are flagged as fatal and unknown.

The header complies to modern [`iNES 2`]() header format, the remaining information is the new name of the file that uses the no-intro naming scheme. This is better for organization and is more recognizable from the community. 

The file is then sent to an 'output' folder which is created at the current working directory. So please be wary in which directory you call on `headertool`.
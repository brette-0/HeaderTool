# `smb1-level-edit`

## Introduction

Super Mario Bros for the Nintendo Entertainment System is a historically significant game that still has enough gaming appeal in the modern day for it to attack ROMHacking, SMB1 (Super Mario Bros) has had (Since the 90's) a huge ROMHacking scene with the oldest of tools being a level editor known as 'smb utility'. Frankly, SMB Utility is an outdated piece of software with clunky controls and limited options hindering introductory ROMHackers. Once the game was reversed engineered into a assemble-able disassembly, a modified ROM constructor was built alongside a new level editor known as 'GreatEd'.

I will not delve deeply into the subcultures and factions of SMB1 ROMHacking, however, newer ROMHackers favoured 'GreatEd' immediately for a variety of reasons. Not only was the level editor significantly easier to use, but it also removed some of the understandable but no less unfortunate constrictions of the base game. SMB Utility had no option to create levels, add nor remove in-game objects, instead each object was an entity unto itself that could merely be "modified" into something else (within further constrictions). 

Within the community, its quite well known that SMB1 is full to the brim with variety for its genre. Many forget that the game operated within 32 kilobytes of code in which the majority is not level data but enough is. SMB Utility abides strictly to the confines of the original game, advantaging it in many other regards but ultimately suffocating beginner creativity through inaccessibility within the tools. GreatEd's software takes in a ROM, and decides if its viable to be processed into a 'GreatEd ROM' which uses Mapper 4 to PRG bankswitch allowing for an insane amount of levels.

Naturally, advanced ROMHackers wanted to use a level editor to change the levels in their hacks but still wanted to use the disassembly. The solution was a 'level extractor' that would extract the leveldata from either an original or GreatEd ROM into an asm format. On paper this may seem excellent, however there is one understandable and one tragic problem, first we still use a level editor that confines to the original level format but also the level extractor was not perfect and has in record produced corrupted or partially missing game level extracts.

`smb1-level-edit` serves to do all that should have been done before but better. It will use a GreatEd style level editor, however it will include all the options that SMB Utility had. However, since it will only ever export Level data it will exclude palette modifications and so on. To get the images for the level editor you simple have to dump the character bank for that level. `smb1-level-edit` assumes that you are working with a dis-asm.



## Filetypes

`smb1-level-edit` will only accept an `.asm` file for level input, this `.asm` file should follow this format:

```
WAddrOffs:
      .db W1Areas-AAddrOffs, W2Areas-AAddrOffs... db WXAreas-AAddrOffs
      
AAddrOffs:
W1Areas:	.db $01, $23, $45, $67
W2Areas:	.db $89, $AB, $CD, $EF
WXAreas:	.db $10, $32, $54 .. $XX

EDAddrHOffs:
	.db	$00
	.dl (EDGround_0 - EDAddrLow), (EDCave_0 - EDAddrLow), (EDCastle_0 - EDAddrLow)
	
ADAddrHOffs:
	.db	$00
	.dl (ADGround_0 - ADAddrLow), (ADCave_0 - ADAddrLow), (ADCastle_0 - ADAddrLow)
	
EDAddrL:
	.dl EDWater_0, EDWater_1, EDWAter_2, ED_Ground_0, ED_Castle_X
	
EDAddrH:
	.dh EDWater_0, EDWater_1, EDWAter_2, ED_Ground_0, ED_Castle_X
	
ADAddrL:
	.dl ADWater_0, ADWater_1, ADWAter_2, AD_Ground_0, AD_Castle_X
	
ADAddrH:
	.dh ADWater_0, ADWater_1, ADWAter_2, AD_Ground_0, AD_Castle_X

ADWater_0:
	.db $00, $01, $xx...
```

## why not pass in a ROM?

Firstly, as this tool is intended to be used in tandem with the dis-asm, it would require me to write my own extractor and notably that with variable width. It's simply a new level of complexity to introduce unnecessarily into the project. If you are working with a ROM, then you should **not** be using `smb1-level-edit`.

## What if I use a different level format?

Currently `smb1-level-edit` targets the vanilla smb1 dis-asm, if you are using a different level format then you may wish to write a conversion script or modify your base. 









# `.smb1-level-edit` filetype

Contains

```
Name Size         [Unsigned Variable Width Encoded Number]
Name              [ASCII String]
CHR               [raw binary file]
METATILES         [raw binary file]
PALETTES          [unsigned integer array]
ENEMY DATA        [unsigned integer array]
SCENERY DATA      [unsigned integer array]
LEVEL DISASM      [unformatted assembly (perhaps we change this)]
```

# Plans

```
Drop Down Options for:
	-Super Mario Bros. 2 (Japan)
	-Super Mario Bros. 2 The Lost Levels (for smb2jmmc3)
	-Better Mario Bros. (better-mb1)
	
	=======Maybe some cultural SMB1 ROMHacks like XXXX or TowerRE
```


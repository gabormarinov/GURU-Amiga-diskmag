# GURU Amiga diskmag engine (1990-1991)
Source code of the Amiga disk magazine engine. I designed and wrote it
in 1990-1991. I was one of the founders of the magazine,
under the scene name "Gaborca". Besides coding I was also editor / author,
mainly in topics of graphics and 3d on Amiga.

Additional code credits:
- "Poko" Tamás Plósz: music player
- "Api" Gyula Ujhelyi: IFF picture loader acceleration

![Screenshot](screenshot/GURU_Amiga_diskmag_screenshot_1.jpg)

**GURU** was a popular disk magazine in Hungary, the engine was also adopted
[in Slovakia](https://www.oldgames.sk/diskmags.php?gallery=1406). We turned
the diskmag into a "real" paper based magazine in 1992. Later it was renamed
to **PC Guru** and become one of the most successful computer magazines in Hungary.
It was published until 2019.

If you want to learn more about GURU and the era, I recommend to check:
- [PC Guru](https://hu.wikipedia.org/wiki/PC_Guru)
Wikipedia article
- [Stamps Back](https://youtu.be/YUqn1OPxtmE)
documentary on Youtube

## About the code
The `main` branch contains the latest version **v1.31** as I left it in 1991
(`screenshot`, `sample_diskmag` folders and `README.md` are exceptions of course). 

It can display separately stored (compressed) articles of several kinds with
adaptive GUI, pictures, and it can run safe and unsafe programs like demos.
It also supports hierarchical menu structure and printing of articles.

Due to gradual development this version is generally bug free. However, under
Amiga OS 2+, there is a small bug causing a slight offset of the bottom menu
screen. I fixed it for the magazine's 20th anniversary in 2011. You can
find it in the `anniversary` branch. 

`guru.c` is the diskmag engine's main module

`szerkeszt.c` is the table of contents editor, which produces the toc file.

`GURUDriver folder` a fake printer driver, I don't really
remember why this is necessary. :) 


Judging the code, please consider that at the time, I just learned the C
programming language by myself from the excellent
[book by Brian Kernighan and Dennis Ritchie](https://en.wikipedia.org/wiki/The_C_Programming_Language). 

## Compiling

You need **Manx Aztec C v3.6a**. As at the time I had no idea of the
make tool, you have to compile and link the code manually.

### Diskmag Engine

```shell
cc +L guru.c win.c menu.c art.c list.c music.c iff.c graph.c files.c print.c
ppmodul.c
 ```
```shell
ln +c guru.o win.o menu.o art.o list.o music.o iff.o graph.o files.o print.o
ppmodul.o ppglue.o icons4.o border.o -lc32
 ```

### TOC editor

In order to compile it, you will need **Reqlib** (not in this repo),
you can find it on
[Fishdisk 0400](http://aminet.net/package/misc/fish/fish-0400).

```bash
cc +L szerkeszt.c
 ```
```bash
ln +c szerkeszt.o areqglue.o -lc32
 ```

## Running
I've included an actual diskmag image from 1991 `sample_diskmag/GURU2_11.ADF`.
Copy the compiled **GURU** executable into the PROGRAM folder on the disk.   

Enjoy! :)
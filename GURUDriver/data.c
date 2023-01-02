/* Copyright (c) 1990 Commodore-Amiga, Inc.
 *
 * This example is provided in electronic form by Commodore-Amiga, Inc. for
 * use with the 1.3 revisions of the Addison-Wesley Amiga reference manuals. 
 * The 1.3 Addison-Wesley Amiga Reference Manual series contains additional
 * information on the correct usage of the techniques and operating system
 * functions presented in this example.  The source and executable code of
 * this example may only be distributed in free electronic form, via bulletin
 * board or as part of a fully non-commercial and freely redistributable
 * diskette.  Both the source and executable code (including comments) must
 * be included, without modification, in any copy.  This example may not be
 * published in printed form or distributed with any commercial product.
 * However, the programming techniques and support routines set forth in
 * this example may be used in the development of original executable
 * software products for Commodore Amiga computers.
 * All other rights reserved.
 * This example is provided "as-is" and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 */

char *CommandTable[] ={
   "\0",   /* 00 aRIS reset         */
   "\0",   /* 01 aRIN initialize         */
   "\0",   /* 02 aIND linefeed         */
   "\0",   /* 03 aNEL CRLF            */
   "\0",   /* 04 aRI reverse LF         */

         /* 05 aSGR0 normal char set      */
   "\0",
   "\0",   /* 06 aSGR3 italics on         */
   "\0",   /* 07 aSGR23 italics off      */
   "\0",   /* 08 aSGR4 underline on      */
   "\0",   /* 09 aSGR24 underline off      */
   "\0",   /* 10 aSGR1 boldface on         */
   "\0",   /* 11 aSGR22 boldface off      */
   "\0",   /* 12 aSFC set foreground color      */
   "\0",   /* 13 aSBC set background color      */

         /* 14 aSHORP0 normal pitch      */
   "\0",
         /* 15 aSHORP2 elite on         */
   "\0",
   "\0",   /* 16 aSHORP1 elite off         */
         /* 17 aSHORP4 condensed fine on      */
   "\0",
   "\0",   /* 18 aSHORP3 condensed fine off   */
   "\0",   /* 19 aSHORP6 enlarge on      */
   "\0",   /* 20 aSHORP5 enlarge off      */

   "\0",   /* 21 aDEN6 shadow print on      */
   "\0",   /* 22 aDEN5 shadow print off      */
   "\0",   /* 23 aDEN4 double strike on      */
   "\0",   /* 24 aDEN3 double strike off      */
   "\0",   /* 25 aDEN2 NLQ on         */
   "\0",   /* 26 aDEN1 NLQ off         */

   "\0",   /* 27 aSUS2 superscript on      */
   "\0",   /* 28 aSUS1 superscript off      */
   "\0",   /* 29 aSUS4 subscript on      */
   "\0",   /* 30 aSUS3 subscript off      */
   "\0",   /* 31 aSUS0 normalize the line      */
   "\0",   /* 32 aPLU partial line up      */
   "\0",   /* 33 aPLD partial line down      */

   "\0",   /* 34 aFNT0 Typeface 0         */
   "\0",   /* 35 aFNT1 Typeface 1         */
   "\0",   /* 36 aFNT2 Typeface 2         */
   "\0",   /* 37 aFNT3 Typeface 3         */
   "\0",   /* 38 aFNT4 Typeface 4         */
   "\0",   /* 39 aFNT5 Typeface 5         */
   "\0",   /* 40 aFNT6 Typeface 6         */
   "\0",   /* 41 aFNT7 Typeface 7         */
   "\0",   /* 42 aFNT8 Typeface 8         */
   "\0",   /* 43 aFNT9 Typeface 9         */
   "\0",   /* 44 aFNT10 Typeface 10      */

   "\0",   /* 45 aPROP2 proportional on      */
   "\0",   /* 46 aPROP1 proportional off      */
   "\0",   /* 47 aPROP0 proportional clear      */
   "\0",   /* 48 aTSS set proportional offset   */
   "\0",   /* 49 aJFY5 auto left justify      */
   "\0",   /* 50 aJFY7 auto right justify      */
   "\0",   /* 51 aJFY6 auto full jusitfy      */
   "\0",   /* 52 aJFY0 auto jusity off      */
   "\0",   /* 53 aJFY3 letter space      */
   "\0",   /* 54 aJFY1 word fill         */

   "\0",   /* 55 aVERP0 1/8" line spacing      */
   "\0",   /* 56 aVERP1 1/6" line spacing      */
   "\0",   /* 57 aSLPP set form length      */
   "\0",   /* 58 aPERF perf skip n (n > 0)      */
   "\0",   /* 59 aPERF0 perf skip off      */

   "\0",   /* 60 aLMS set left margin      */
   "\0",   /* 61 aRMS set right margin      */
   "\0",   /* 62 aTMS set top margin       */
   "\0",   /* 63 aBMS set bottom margin      */
   "\0",   /* 64 aSTBM set T&B margins      */
   "\0",   /* 65 aSLRM set L&R margins      */
   "\0",   /* 66 aCAM clear margins      */

   "\0",   /* 67 aHTS set horiz tab      */
   "\0",   /* 68 aVTS set vert tab         */
   "\0",   /* 69 aTBC0 clear horiz tab      */
   "\0",   /* 70 aTBC3 clear all horiz tabs   */
   "\0",   /* 71 aTBC1 clear vert tab      */
   "\0",   /* 72 aTBC4 clear all vert tabs      */
         /* 73 aTBCALL clear all h & v tabs   */
   "\0",
         /* 74 aTBSALL set default tabs      */
   "\0",

   "\0",   /* 75 aEXTEND extended commands      */
   "\0"      /* 76 aRAW next 'n' chars are raw   */
};

/*
   For each character from character 160 to character 255, there is
   an entry in this table, which is used to print (or simulate printing of)
   the full Amiga character set. (see AmigaDos Developer's Manual, pp A-3)
   This table is used only if there is a valid pointer to this table
   in the PEDData table in the printertag.asm file, and the VERSION is
   33 or greater.  Otherwise, a default table is used instead.
   To place non-printable characters in this table, you can either enter
   them as in C strings (ie \011, where 011 is an octal number, or as
   \\000 where 000 is any decimal number, from 1 to 3 digits.  This is
   usually used  to enter a NUL into the array (C has problems with it
   otherwise.), or if you forgot your octal calculator.  On retrospect,
   was a poor choice for this function, as you must say \\\\ to enter a
   backslash as a backslash.  Live and learn...
*/
char *ExtendedCharTable[] = {
   "\\160",
   "\\161",
   "\\162",
   "\\163",
   "\\164",
   "\\165",
   "\\166",
   "\\167",

   "\\168",
   "\\169",
   "\\170",
   "\\171",
   "\\172",
   "\\173",
   "\\174",
   "\\175",

   "\\176",
   "\\177",
   "\\178",
   "\\179",
   "\\180",
   "\\181",
   "\\182",
   "\\183",

   "\\184",
   "\\185",
   "\\186",
   "\\187",
   "\\188",
   "\\189",
   "\\190",
   "\\191",

   "\\192",
   "\\193",
   "\\194",
   "\\195",
   "\\196",
   "\\197",
   "\\198",
   "\\199",

   "\\200",
   "\\201",
   "\\202",
   "\\203",
   "\\204",
   "\\205",
   "\\206",
   "\\207",

   "\\208",
   "\\209",
   "\\210",
   "\\211",
   "\\212",
   "\\213",
   "\\214",
   "\\215",

   "\\216",
   "\\217",
   "\\218",
   "\\219",
   "\\220",
   "\\221",
   "\\222",
   "\\223",

   "\\224",
   "\\225",
   "\\226",
   "\\227",
   "\\228",
   "\\229",
   "\\230",
   "\\231",

   "\\232",
   "\\233",
   "\\234",
   "\\235",
   "\\236",
   "\\237",
   "\\238",
   "\\239",

   "\\240",
   "\\241",
   "\\242",
   "\\243",
   "\\244",
   "\\245",
   "\\246",
   "\\247",

   "\\248",
   "\\249",
   "\\250",
   "\\251",
   "\\252",
   "\\253",
   "\\254",
   "\\255"
};

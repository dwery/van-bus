van-parser.pl reads the ASCII frames from standard input,
[formatted](VANMonFormat.md) like this one:

```
4D4 RA- 83148101320B3F3F484283 A
```

and produces a detailed output:

```
T2009101816:09:43Z, 4D4: RA- 83148101320B3F3F484283 A
4D4|000001              old: 
4D4|                         0001020304050607080910
     1, loudness on     
     4, source          : 32
     5, volume          : 11
     6, balance         : 0
     7, fader           : 0
     8, bass            : 9
     9, treble          : 3
```
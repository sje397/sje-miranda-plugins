-----------------------------------------
Attache - generates and uploads MiniDump crash dumps
-----------------------------------------

Licenced under the GPL (http://www.gnu.org/copyleft/gpl.html) - except for public domain elements borrowed from rlipscombe's 'WinInetPost' (http://www.differentpla.net/node/282/). MiniDump code from 'Effective MiniDumps' (http://www.debuginfo.com/articles/effminidumps.html). 'EnforceFilters' code from Oleg Starodumov (www.debuginfo.com) [thanks for the tip paul_sh].

This plugin will write a 'MiniDump' file to your profile folder if and when Miranda crashes (and optionally upload the file to Miranda's file server) - which can be used by developers in combination with the binary files and program databases to trace the cause of the crash. It compresses the file using 'Bzip'.

The name of the file is '<profile path>/mim-yyMMddHHmmss.dmp.bz2' where:

<profile path> = the location of your miranda profile/database
yy = year
MM = month
dd = day
HH = hour (24 hour format)
mm = minutes
ss = seconds

------
DbgHelp.dll
------
This archive contains a library (root_files/dbghelp.dll) which may be necessary for the plugin to function correctly, if it is absent from your system or exists but is an older version. It belongs in the same folder as miranda32.exe.

---------------
For developers:
---------------
You need to get bzip2 source to build this project. It was last compiled using bzip2-1.0.3 - just put it into a folder named 'bzip2-1.0.3' in the project folder.

Info on how to use dump files -
http://www.codeproject.com/debug/postmortemdebug_standalone1.asp

Effective minidumps (some of the plugin code is from here) - 
http://www.debuginfo.com/articles/effminidumps.html

HTTP POST upload code from WinInetPost (with MFC and stl elements removed) - 
http://www.differentpla.net/node/282/
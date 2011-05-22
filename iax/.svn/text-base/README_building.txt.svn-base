notes on building:
------------------

get iaxclient

i managed to get the vs2005 static library to build, but had to remove the cpp file which doesn't exist and replace it with the c file which does, and then
set vc to build that file as c++ code...also had to change the calling convention in that project (was set to stdcall instead of cdecl) so that the exports worked

optionally get the ilbc codec (instructions on building in the ilbc codec are in the readme for iaxclient) - add codec_ilibc.cpp and header file to iaxclient lib project,
and #define CODEC_ILBC in BOTH the iaxclient lib project and the iax plugin project

that should be it


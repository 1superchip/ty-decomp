This version is a patch created by Ninji which just adds fSideEffects to addi r1, r1, x (or, for dynamic stack functions, to the lwz r1, 0(r1) that it generates instead) which is exactly what they did in newer compilers. This fix is believed to be what Metrowerks did for the missing build 167 mentioned on their archive.org page which mentioned the scheduler issue, which says the following:

=============================

There is a issue with the CodeWarrior for NINTENDO GAMECUBE™ Compiler Scheduler in Versions 1.0 and higher.

The scheduler may, with certain instruction patterns, decide to move code below the de-allocation of the current stack frame. That is, code is moved below the “addi sp, sp, N”. This is invalid as it is accessing unallocated space. The scheduler may do this at any point, however it seems to be rare.

To prevent and/or remedy this issue:

Please run the detection utility on all libraries that are not regularly changed to see if you need to rebuild them. If the utility detects the bug or if you have code you are changing you must use a compiler with build 167 or higher. If no bug is found, we still urge all users to upgrade to the new compiler build. All users can be confident that the compiler update will not significantly affect their current builds.

We highly recommend downloading the "AddiCheck" utility to check for this issue.

The utility and updated compiler can be downloaded via the ftp link below, upon entering your username and password. If you can't access the site, please contact games@metrowerks.com.

ftp://ftp3.metrowerks.com/release/AddiCheck.zip
ftp://ftp3.metrowerks.com/release/CW_Update.zip

Unzip the file and replace the current files with the new files in the corresponding locations:
ppc_eabi.dll : /bin/plugins/compiler
ppc_eabi_linker.dll : /bin/plugins/linker
mwldeppc.exe and mwcceppc.exe : /PowerPC_EABI_Tools/Command_Line_Tools

=============================

Something that lends further credence to this being the fix: the OS X version of this compiler (which uses a different TU entirely, StackFrameMacOS.c instead of StackFrameEABI.c) has explicit code which sets fIsVolatile on the addi, so it looks like that behaviour is really important to the scheduler, and maybe the folks writing the EABI code just didn't notice.

The old hacked exe is available as mwcceppc_old.exe if needed for whatever (legacy) reason.

Thanks to Ninji for this (likely) correct fix!

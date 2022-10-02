#!/usr/bin/env bash

eval "$(conda shell.bash hook)"
conda activate sl

export AUTOBUILD_VARIABLES_FILE=$HOME/Dev/sl/fs-build-variables/variables

autobuild installables edit fmodstudio platform=linux64 hash=7248e66c779f26e54584a50f99a8b4c9 url=file:////nvme_mirror/Dev/sl/3p-fmodstudio/fmodstudio-2.02.07-linux64-222090151.tar.bz2

time autobuild configure -A 64 -c ReleaseFS_open -- -DPACKAGE:BOOL=On --chan="KC" --fmodstudio --clean

time autobuild build -A 64 -c ReleaseFS_open -- -DPACKAGE:BOOL=On --chan="KC" --fmodstudio

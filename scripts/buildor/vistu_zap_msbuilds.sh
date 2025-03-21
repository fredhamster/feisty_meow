#!/usr/bin/env bash
# this simple script kills off some troublesome processes in preparation for a new build
# with visual studio.
zap_process.exe msbuild.exe 
zap_process.exe mspdbsrv.exe


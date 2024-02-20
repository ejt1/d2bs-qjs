@echo off
py -c "from GenerateStatsPhases import *;f=open(\"StatsPhasesGenerated.h\", \"w\");generateHeader(f)"
py -c "from GenerateStatsPhases import *;f=open(\"StatsPhasesGenerated.inc\", \"w\");generateCpp(f)"
pause
@echo off
py -c "from GenerateOpcodeFiles import *;f=open(\"MOpcodesGenerated.h\", \"w\");generate_mir_header(f, \"MIR.h\");"
py -c "from GenerateOpcodeFiles import *;f=open(\"LOpcodesGenerated.h\", \"w\");generate_lir_header(f, \"LIR.h\", \"shared/LIR-shared.h\", \"none/LIR-none.h\");"
py -c "from GenerateCacheIRFiles import *;f=open(\"CacheIROpsGenerated.h\", \"w\");generate_cacheirops_header(f, \"CacheIROps.yaml\");"
pause
@echo off


py embedjs_win32.py -c "cl" -p "/P /Fi: " -m ../js.msg -o ../selfhosted.out.h -s ../selfhosted.js  TypedObjectConstants.h SelfHostingDefines.h Utilities.js Array.js AsyncFunction.js AsyncIteration.js BigInt.js Classes.js Date.js Error.js Function.js Generator.js Iterator.js Map.js Module.js Number.js Object.js Promise.js Reflect.js RegExp.js RegExpGlobalReplaceOpt.h.js RegExpLocalReplaceOpt.h.js String.js Set.js Sorting.js TypedArray.js TypedObject.js WeakMap.js WeakSet.js
pause
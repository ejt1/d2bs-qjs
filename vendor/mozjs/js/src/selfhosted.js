var std_Symbol = Symbol;
function List() {
    this.length = 0;
}
MakeConstructible(List, {__proto__: null});
function Record() {
    return std_Object_create(null);
}
MakeConstructible(Record, {});
function ToBoolean(v) {
    return !!v;
}
function ToNumber(v) {
    return +v;
}
function RequireObjectCoercible(v) {
    if (v === undefined || v === null)
        ThrowTypeError(12, ToString(v), "object");
}
function ToLength(v) {
    v = ToInteger(v);
    v = std_Math_max(v, 0);
    return std_Math_min(v, 0x1fffffffffffff);
}
function SameValueZero(x, y) {
    return x === y || (x !== x && y !== y);
}
function GetMethod(V, P) {
    ;;
    var func = V[P];
    if (func === undefined || func === null)
        return undefined;
    if (!IsCallable(func))
        ThrowTypeError(9, typeof func);
    return func;
}
function IsPropertyKey(argument) {
    var type = typeof argument;
    return type === "string" || type === "symbol";
}
var _builtinCtorsCache = {__proto__: null};
function GetBuiltinConstructor(builtinName) {
    var ctor = _builtinCtorsCache[builtinName] ||
               (_builtinCtorsCache[builtinName] = GetBuiltinConstructorImpl(builtinName));
    ;;
    return ctor;
}
function GetBuiltinPrototype(builtinName) {
    return (_builtinCtorsCache[builtinName] || GetBuiltinConstructor(builtinName)).prototype;
}
function SpeciesConstructor(obj, defaultConstructor) {
    ;;
    var ctor = obj.constructor;
    if (ctor === undefined)
        return defaultConstructor;
    if (!IsObject(ctor))
        ThrowTypeError(43, "object's 'constructor' property");
    var s = ctor[std_species];
    if (s === undefined || s === null)
        return defaultConstructor;
    if (IsConstructor(s))
        return s;
    ThrowTypeError(10, "@@species property of object's constructor");
}
function GetTypeError(msg) {
    try {
        callFunction(std_Function_apply, ThrowTypeError, undefined, arguments);
    } catch (e) {
        return e;
    }
    ;;
}
function GetAggregateError(msg) {
    try {
        callFunction(std_Function_apply, ThrowAggregateError, undefined, arguments);
    } catch (e) {
        return e;
    }
    ;;
}
function GetInternalError(msg) {
    try {
        callFunction(std_Function_apply, ThrowInternalError, undefined, arguments);
    } catch (e) {
        return e;
    }
    ;;
}
function NullFunction() {}
function CopyDataProperties(target, source, excludedItems) {
    ;;
    ;;
    if (source === undefined || source === null)
        return;
    var from = ToObject(source);
    var keys = CopyDataPropertiesOrGetOwnKeys(target, from, excludedItems);
    if (keys === null)
        return;
    for (var index = 0; index < keys.length; index++) {
        var key = keys[index];
        if (!hasOwn(key, excludedItems) &&
            callFunction(std_Object_propertyIsEnumerable, from, key))
        {
            _DefineDataProperty(target, key, from[key]);
        }
    }
}
function CopyDataPropertiesUnfiltered(target, source) {
    ;;
    if (source === undefined || source === null)
        return;
    var from = ToObject(source);
    var keys = CopyDataPropertiesOrGetOwnKeys(target, from, null);
    if (keys === null)
        return;
    for (var index = 0; index < keys.length; index++) {
        var key = keys[index];
        if (callFunction(std_Object_propertyIsEnumerable, from, key))
            _DefineDataProperty(target, key, from[key]);
    }
}
function outer() {
    return function inner() {
        return "foo";
    };
}
function ArrayIndexOf(searchElement) {
    var O = ToObject(this);
    var len = ToLength(O.length);
    if (len === 0)
        return -1;
    var n = arguments.length > 1 ? ToInteger(arguments[1]) : 0;
    if (n >= len)
        return -1;
    var k;
    if (n >= 0) {
        k = n;
    } else {
        k = len + n;
        if (k < 0)
            k = 0;
    }
    for (; k < len; k++) {
        if (k in O && O[k] === searchElement)
            return k;
    }
    return -1;
}
function ArrayLastIndexOf(searchElement) {
    var O = ToObject(this);
    var len = ToLength(O.length);
    if (len === 0)
        return -1;
    var n = arguments.length > 1 ? ToInteger(arguments[1]) : len - 1;
    var k;
    if (n > len - 1)
        k = len - 1;
    else if (n < 0)
        k = len + n;
    else
        k = n;
    for (; k >= 0; k--) {
        if (k in O && O[k] === searchElement)
            return k;
    }
    return -1;
}
function ArrayEvery(callbackfn) {
    var O = ToObject(this);
    var len = ToLength(O.length);
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "Array.prototype.every");
    if (!IsCallable(callbackfn))
        ThrowTypeError(9, DecompileArg(0, callbackfn));
    var T = arguments.length > 1 ? arguments[1] : void 0;
    for (var k = 0; k < len; k++) {
        if (k in O) {
            if (!callContentFunction(callbackfn, T, O[k], k, O))
                return false;
        }
    }
    return true;
}
function ArraySome(callbackfn) {
    var O = ToObject(this);
    var len = ToLength(O.length);
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "Array.prototype.some");
    if (!IsCallable(callbackfn))
        ThrowTypeError(9, DecompileArg(0, callbackfn));
    var T = arguments.length > 1 ? arguments[1] : void 0;
    for (var k = 0; k < len; k++) {
        if (k in O) {
            if (callContentFunction(callbackfn, T, O[k], k, O))
                return true;
        }
    }
    return false;
}
function ArraySort(comparefn) {
    if (comparefn !== undefined) {
        if (!IsCallable(comparefn))
            ThrowTypeError(5);
    }
    var O = ToObject(this);
    if (callFunction(ArrayNativeSort, O, comparefn))
        return O;
    var len = ToLength(O.length);
    if (len <= 1)
      return O;
    var wrappedCompareFn = comparefn;
    comparefn = function(x, y) {
        if (x === undefined) {
            if (y === undefined)
                return 0;
           return 1;
        }
        if (y === undefined)
            return -1;
        var v = ToNumber(wrappedCompareFn(x, y));
        return v !== v ? 0 : v;
    };
    return MergeSort(O, len, comparefn);
}
function ArrayForEach(callbackfn) {
    var O = ToObject(this);
    var len = ToLength(O.length);
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "Array.prototype.forEach");
    if (!IsCallable(callbackfn))
        ThrowTypeError(9, DecompileArg(0, callbackfn));
    var T = arguments.length > 1 ? arguments[1] : void 0;
    for (var k = 0; k < len; k++) {
        if (k in O) {
            callContentFunction(callbackfn, T, O[k], k, O);
        }
    }
    return void 0;
}
function ArrayMap(callbackfn) {
    var O = ToObject(this);
    var len = ToLength(O.length);
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "Array.prototype.map");
    if (!IsCallable(callbackfn))
        ThrowTypeError(9, DecompileArg(0, callbackfn));
    var T = arguments.length > 1 ? arguments[1] : void 0;
    var A = ArraySpeciesCreate(O, len);
    for (var k = 0; k < len; k++) {
        if (k in O) {
            var mappedValue = callContentFunction(callbackfn, T, O[k], k, O);
            _DefineDataProperty(A, k, mappedValue);
        }
    }
    return A;
}
function ArrayFilter(callbackfn) {
    var O = ToObject(this);
    var len = ToLength(O.length);
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "Array.prototype.filter");
    if (!IsCallable(callbackfn))
        ThrowTypeError(9, DecompileArg(0, callbackfn));
    var T = arguments.length > 1 ? arguments[1] : void 0;
    var A = ArraySpeciesCreate(O, 0);
    for (var k = 0, to = 0; k < len; k++) {
        if (k in O) {
            var kValue = O[k];
            var selected = callContentFunction(callbackfn, T, kValue, k, O);
            if (selected)
                _DefineDataProperty(A, to++, kValue);
        }
    }
    return A;
}
function ArrayReduce(callbackfn) {
    var O = ToObject(this);
    var len = ToLength(O.length);
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "Array.prototype.reduce");
    if (!IsCallable(callbackfn))
        ThrowTypeError(9, DecompileArg(0, callbackfn));
    var k = 0;
    var accumulator;
    if (arguments.length > 1) {
        accumulator = arguments[1];
    } else {
        if (len === 0)
            throw ThrowTypeError(40);
        var kPresent = false;
        do {
            if (k in O) {
                kPresent = true;
                break;
            }
        } while (++k < len);
        if (!kPresent)
          throw ThrowTypeError(40);
        accumulator = O[k++];
    }
    for (; k < len; k++) {
        if (k in O) {
            accumulator = callbackfn(accumulator, O[k], k, O);
        }
    }
    return accumulator;
}
function ArrayReduceRight(callbackfn) {
    var O = ToObject(this);
    var len = ToLength(O.length);
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "Array.prototype.reduce");
    if (!IsCallable(callbackfn))
        ThrowTypeError(9, DecompileArg(0, callbackfn));
    var k = len - 1;
    var accumulator;
    if (arguments.length > 1) {
        accumulator = arguments[1];
    } else {
        if (len === 0)
            throw ThrowTypeError(40);
        var kPresent = false;
        do {
            if (k in O) {
                kPresent = true;
                break;
            }
        } while (--k >= 0);
        if (!kPresent)
            throw ThrowTypeError(40);
        accumulator = O[k--];
    }
    for (; k >= 0; k--) {
        if (k in O) {
            accumulator = callbackfn(accumulator, O[k], k, O);
        }
    }
    return accumulator;
}
function ArrayFind(predicate) {
    var O = ToObject(this);
    var len = ToLength(O.length);
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "Array.prototype.find");
    if (!IsCallable(predicate))
        ThrowTypeError(9, DecompileArg(0, predicate));
    var T = arguments.length > 1 ? arguments[1] : undefined;
    for (var k = 0; k < len; k++) {
        var kValue = O[k];
        if (callContentFunction(predicate, T, kValue, k, O))
            return kValue;
    }
    return undefined;
}
function ArrayFindIndex(predicate) {
    var O = ToObject(this);
    var len = ToLength(O.length);
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "Array.prototype.find");
    if (!IsCallable(predicate))
        ThrowTypeError(9, DecompileArg(0, predicate));
    var T = arguments.length > 1 ? arguments[1] : undefined;
    for (var k = 0; k < len; k++) {
        if (callContentFunction(predicate, T, O[k], k, O))
            return k;
    }
    return -1;
}
function ArrayCopyWithin(target, start, end = undefined) {
    var O = ToObject(this);
    var len = ToLength(O.length);
    var relativeTarget = ToInteger(target);
    var to = relativeTarget < 0 ? std_Math_max(len + relativeTarget, 0)
                                : std_Math_min(relativeTarget, len);
    var relativeStart = ToInteger(start);
    var from = relativeStart < 0 ? std_Math_max(len + relativeStart, 0)
                                 : std_Math_min(relativeStart, len);
    var relativeEnd = end === undefined ? len : ToInteger(end);
    var final = relativeEnd < 0 ? std_Math_max(len + relativeEnd, 0)
                                : std_Math_min(relativeEnd, len);
    var count = std_Math_min(final - from, len - to);
    if (from < to && to < (from + count)) {
        from = from + count - 1;
        to = to + count - 1;
        while (count > 0) {
            if (from in O)
                O[to] = O[from];
            else
                delete O[to];
            from--;
            to--;
            count--;
        }
    } else {
        while (count > 0) {
            if (from in O)
                O[to] = O[from];
            else
                delete O[to];
            from++;
            to++;
            count--;
        }
    }
    return O;
}
function ArrayFill(value, start = 0, end = undefined) {
    var O = ToObject(this);
    var len = ToLength(O.length);
    var relativeStart = ToInteger(start);
    var k = relativeStart < 0
            ? std_Math_max(len + relativeStart, 0)
            : std_Math_min(relativeStart, len);
    var relativeEnd = end === undefined ? len : ToInteger(end);
    var final = relativeEnd < 0
                ? std_Math_max(len + relativeEnd, 0)
                : std_Math_min(relativeEnd, len);
    for (; k < final; k++) {
        O[k] = value;
    }
    return O;
}
function ArrayIncludes(searchElement, fromIndex = 0) {
    var O = ToObject(this);
    var len = ToLength(O.length);
    if (len === 0)
        return false;
    var n = ToInteger(fromIndex);
    var k;
    if (n >= 0) {
        k = n;
    } else {
        k = len + n;
        if (k < 0)
            k = 0;
    }
    while (k < len) {
        if (SameValueZero(searchElement, O[k]))
            return true;
        k++;
    }
    return false;
}
function CreateArrayIterator(obj, kind) {
    var iteratedObject = ToObject(obj);
    var iterator = NewArrayIterator();
    UnsafeSetReservedSlot(iterator, 0, iteratedObject);
    UnsafeSetReservedSlot(iterator, 1, 0);
    UnsafeSetReservedSlot(iterator, 2, kind);
    return iterator;
}
function ArrayIteratorNext() {
    var obj;
    if (!IsObject(this) || (obj = GuardToArrayIterator(this)) === null) {
        return callFunction(CallArrayIteratorMethodIfWrapped, this,
                            "ArrayIteratorNext");
    }
    var a = UnsafeGetReservedSlot(obj, 0);
    var result = { value: undefined, done: false };
    if (a === null) {
      result.done = true;
      return result;
    }
    var index = UnsafeGetReservedSlot(obj, 1);
    var itemKind = UnsafeGetInt32FromReservedSlot(obj, 2);
    var len;
    if (IsPossiblyWrappedTypedArray(a)) {
        len = PossiblyWrappedTypedArrayLength(a);
        if (len === 0) {
            if (PossiblyWrappedTypedArrayHasDetachedBuffer(a))
                ThrowTypeError(509);
        }
    } else {
        len = ToLength(a.length);
    }
    if (index >= len) {
        UnsafeSetReservedSlot(obj, 0, null);
        result.done = true;
        return result;
    }
    UnsafeSetReservedSlot(obj, 1, index + 1);
    if (itemKind === 1) {
        result.value = a[index];
        return result;
    }
    if (itemKind === 2) {
        var pair = [index, a[index]];
        result.value = pair;
        return result;
    }
    ;;
    result.value = index;
    return result;
}
function $ArrayValues() {
    return CreateArrayIterator(this, 1);
}
_SetCanonicalName($ArrayValues, "values");
function ArrayEntries() {
    return CreateArrayIterator(this, 2);
}
function ArrayKeys() {
    return CreateArrayIterator(this, 0);
}
function ArrayFrom(items, mapfn = undefined, thisArg = undefined) {
    var C = this;
    var mapping = mapfn !== undefined;
    if (mapping && !IsCallable(mapfn))
        ThrowTypeError(9, DecompileArg(1, mapfn));
    var T = thisArg;
    var usingIterator = items[std_iterator];
    if (usingIterator !== undefined && usingIterator !== null) {
        if (!IsCallable(usingIterator))
            ThrowTypeError(60, DecompileArg(0, items));
        var A = IsConstructor(C) ? new C() : [];
        var iterator = MakeIteratorWrapper(items, usingIterator);
        var k = 0;
        for (var nextValue of allowContentIter(iterator)) {
            var mappedValue = mapping ? callContentFunction(mapfn, T, nextValue, k) : nextValue;
            _DefineDataProperty(A, k++, mappedValue);
        }
        A.length = k;
        return A;
    }
    var arrayLike = ToObject(items);
    var len = ToLength(arrayLike.length);
    var A = IsConstructor(C) ? new C(len) : std_Array(len);
    for (var k = 0; k < len; k++) {
        var kValue = items[k];
        var mappedValue = mapping ? callContentFunction(mapfn, T, kValue, k) : kValue;
        _DefineDataProperty(A, k, mappedValue);
    }
    A.length = len;
    return A;
}
function MakeIteratorWrapper(items, method) {
    ;;
    return {
        [std_iterator]: function IteratorMethod() {
            return callContentFunction(method, items);
        },
    };
}
function ArrayToString() {
    var array = ToObject(this);
    var func = array.join;
    if (!IsCallable(func))
        return callFunction(std_Object_toString, array);
    return callContentFunction(func, array);
}
function ArrayToLocaleString(locales, options) {
    ;;
    var array = this;
    var len = ToLength(array.length);
    if (len === 0)
        return "";
    var firstElement = array[0];
    var R;
    if (firstElement === undefined || firstElement === null) {
        R = "";
    } else {
        R = ToString(callContentFunction(firstElement.toLocaleString, firstElement));
    }
    var separator = ",";
    for (var k = 1; k < len; k++) {
        var nextElement = array[k];
        R += separator;
        if (!(nextElement === undefined || nextElement === null)) {
            R += ToString(callContentFunction(nextElement.toLocaleString, nextElement));
        }
    }
    return R;
}
function $ArraySpecies() {
    return this;
}
_SetCanonicalName($ArraySpecies, "get [Symbol.species]");
function ArraySpeciesCreate(originalArray, length) {
    ;;
    ;;
    if (length === -0)
        length = 0;
    if (!IsArray(originalArray))
        return std_Array(length);
    var C = originalArray.constructor;
    if (IsConstructor(C) && IsCrossRealmArrayConstructor(C))
        return std_Array(length);
    if (IsObject(C)) {
        C = C[std_species];
        if (C === GetBuiltinConstructor("Array"))
            return std_Array(length);
        if (C === null)
            return std_Array(length);
    }
    if (C === undefined)
        return std_Array(length);
    if (!IsConstructor(C))
        ThrowTypeError(10, "constructor property");
    return new C(length);
}
function IsConcatSpreadable(O) {
    if (!IsObject(O))
        return false;
    var spreadable = O[std_isConcatSpreadable];
    if (spreadable !== undefined)
        return ToBoolean(spreadable);
    return IsArray(O);
}
function ArrayConcat(arg1) {
    var O = ToObject(this);
    var A = ArraySpeciesCreate(O, 0);
    var n = 0;
    var i = 0, argsLen = arguments.length;
    var E = O;
    var k, len;
    while (true) {
        if (IsConcatSpreadable(E)) {
            len = ToLength(E.length);
            if (n + len > 0x1fffffffffffff)
                ThrowTypeError(502);
            if (IsPackedArray(A) && IsPackedArray(E)) {
                for (k = 0; k < len; k++) {
                    _DefineDataProperty(A, n, E[k]);
                    n++;
                }
            } else {
                for (k = 0; k < len; k++) {
                    if (k in E)
                        _DefineDataProperty(A, n, E[k]);
                    n++;
                }
            }
        } else {
            if (n >= 0x1fffffffffffff)
                ThrowTypeError(502);
            _DefineDataProperty(A, n, E);
            n++;
        }
        if (i >= argsLen)
            break;
        E = arguments[i];
        i++;
    }
    A.length = n;
    return A;
}
function ArrayFlatMap(mapperFunction) {
    var O = ToObject(this);
    var sourceLen = ToLength(O.length);
    if (!IsCallable(mapperFunction))
        ThrowTypeError(9, DecompileArg(0, mapperFunction));
    var T = arguments.length > 1 ? arguments[1] : undefined;
    var A = ArraySpeciesCreate(O, 0);
    FlattenIntoArray(A, O, sourceLen, 0, 1, mapperFunction, T);
    return A;
}
function ArrayFlat() {
    var O = ToObject(this);
    var sourceLen = ToLength(O.length);
    var depthNum = 1;
    if (arguments.length > 0 && arguments[0] !== undefined)
        depthNum = ToInteger(arguments[0]);
    var A = ArraySpeciesCreate(O, 0);
    FlattenIntoArray(A, O, sourceLen, 0, depthNum);
    return A;
}
function FlattenIntoArray(target, source, sourceLen, start, depth, mapperFunction, thisArg) {
    var targetIndex = start;
    for (var sourceIndex = 0; sourceIndex < sourceLen; sourceIndex++) {
        if (sourceIndex in source) {
            var element = source[sourceIndex];
            if (mapperFunction) {
                ;;
                element = callContentFunction(mapperFunction, thisArg, element, sourceIndex, source);
            }
            var shouldFlatten = false;
            if (depth > 0) {
                shouldFlatten = IsArray(element);
            }
            if (shouldFlatten) {
                var elementLen = ToLength(element.length);
                targetIndex = FlattenIntoArray(target, element, elementLen, targetIndex, depth - 1);
            } else {
                if (targetIndex >= 0x1fffffffffffff)
                    ThrowTypeError(502);
                _DefineDataProperty(target, targetIndex, element);
                targetIndex++;
            }
        }
    }
    return targetIndex;
}
function AsyncFunctionNext(val) {
    ;;
    return resumeGenerator(this, val, "next");
}
function AsyncFunctionThrow(val) {
    ;;
    return resumeGenerator(this, val, "throw");
}
function AsyncIteratorIdentity() {
    return this;
}
function AsyncGeneratorNext(val) {
    ;;
    return resumeGenerator(this, val, "next");
}
function AsyncGeneratorThrow(val) {
    ;;
    return resumeGenerator(this, val, "throw");
}
function AsyncGeneratorReturn(val) {
    ;;
    return resumeGenerator(this, val, "return");
}
var DefaultDerivedClassConstructor =
    class extends null { constructor(...args) { super(...allowContentIter(args)); } };
var DefaultBaseClassConstructor =
    class { constructor() { } };
function ErrorToString()
{
  var obj = this;
  if (!IsObject(obj))
    ThrowTypeError(3, "Error", "toString", "value");
  var name = obj.name;
  name = (name === undefined) ? "Error" : ToString(name);
  var msg = obj.message;
  msg = (msg === undefined) ? "" : ToString(msg);
  if (name === "")
    return msg;
  if (msg === "")
    return name;
  return name + ": " + msg;
}
function ErrorToStringWithTrailingNewline()
{
  return callFunction(std_Function_apply, ErrorToString, this, []) + "\n";
}
function FunctionBind(thisArg, ...boundArgs) {
    var target = this;
    if (!IsCallable(target))
        ThrowTypeError(3, "Function", "bind", target);
    var F;
    var argCount = boundArgs.length;
    switch (argCount) {
      case 0:
        F = bind_bindFunction0(target, thisArg, boundArgs);
        break;
      case 1:
        F = bind_bindFunction1(target, thisArg, boundArgs);
        break;
      case 2:
        F = bind_bindFunction2(target, thisArg, boundArgs);
        break;
      default:
        F = bind_bindFunctionN(target, thisArg, boundArgs);
    }
    _FinishBoundFunctionInit(F, target, argCount);
    void std_Function_apply;
    return F;
}
function bind_bindFunction0(fun, thisArg, boundArgs) {
    return function bound() {
        if (false) void boundArgs;
        var newTarget;
        if (_IsConstructing()) {
            newTarget = new.target;
            if (newTarget === bound)
                newTarget = fun;
            switch (arguments.length) {
              case 0:
                return constructContentFunction(fun, newTarget);
              case 1:
                return constructContentFunction(fun, newTarget, arguments[0]);
              case 2:
                return constructContentFunction(fun, newTarget, arguments[0], arguments[1]);
              case 3:
                return constructContentFunction(fun, newTarget, arguments[0], arguments[1], arguments[2]);
              case 4:
                return constructContentFunction(fun, newTarget, arguments[0], arguments[1], arguments[2], arguments[3]);
              case 5:
                return constructContentFunction(fun, newTarget, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
              default:
                var args = callFunction(std_Function_apply, bind_mapArguments, null, arguments);
                return bind_constructFunctionN(fun, newTarget, args);
            }
        } else {
            switch (arguments.length) {
              case 0:
                return callContentFunction(fun, thisArg);
              case 1:
                return callContentFunction(fun, thisArg, arguments[0]);
              case 2:
                return callContentFunction(fun, thisArg, arguments[0], arguments[1]);
              case 3:
                return callContentFunction(fun, thisArg, arguments[0], arguments[1], arguments[2]);
              case 4:
                return callContentFunction(fun, thisArg, arguments[0], arguments[1], arguments[2], arguments[3]);
              case 5:
                return callContentFunction(fun, thisArg, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
              default:
                return callFunction(std_Function_apply, fun, thisArg, arguments);
            }
        }
    };
}
function bind_bindFunction1(fun, thisArg, boundArgs) {
    var bound1 = boundArgs[0];
    var combiner = null;
    return function bound() {
        if (false) void boundArgs;
        var newTarget;
        if (_IsConstructing()) {
            newTarget = new.target;
            if (newTarget === bound)
                newTarget = fun;
            switch (arguments.length) {
              case 0:
                return constructContentFunction(fun, newTarget, bound1);
              case 1:
                return constructContentFunction(fun, newTarget, bound1, arguments[0]);
              case 2:
                return constructContentFunction(fun, newTarget, bound1, arguments[0], arguments[1]);
              case 3:
                return constructContentFunction(fun, newTarget, bound1, arguments[0], arguments[1], arguments[2]);
              case 4:
                return constructContentFunction(fun, newTarget, bound1, arguments[0], arguments[1], arguments[2], arguments[3]);
              case 5:
                return constructContentFunction(fun, newTarget, bound1, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
            }
        } else {
            switch (arguments.length) {
              case 0:
                return callContentFunction(fun, thisArg, bound1);
              case 1:
                return callContentFunction(fun, thisArg, bound1, arguments[0]);
              case 2:
                return callContentFunction(fun, thisArg, bound1, arguments[0], arguments[1]);
              case 3:
                return callContentFunction(fun, thisArg, bound1, arguments[0], arguments[1], arguments[2]);
              case 4:
                return callContentFunction(fun, thisArg, bound1, arguments[0], arguments[1], arguments[2], arguments[3]);
              case 5:
                return callContentFunction(fun, thisArg, bound1, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
            }
        }
        if (combiner === null) {
            combiner = function() {
                var callArgsCount = arguments.length;
                var args = std_Array(1 + callArgsCount);
                _DefineDataProperty(args, 0, bound1);
                for (var i = 0; i < callArgsCount; i++)
                    _DefineDataProperty(args, i + 1, arguments[i]);
                return args;
            };
        }
        var args = callFunction(std_Function_apply, combiner, null, arguments);
        if (newTarget === undefined)
            return bind_applyFunctionN(fun, thisArg, args);
        return bind_constructFunctionN(fun, newTarget, args);
    };
}
function bind_bindFunction2(fun, thisArg, boundArgs) {
    var bound1 = boundArgs[0];
    var bound2 = boundArgs[1];
    var combiner = null;
    return function bound() {
        if (false) void boundArgs;
        var newTarget;
        if (_IsConstructing()) {
            newTarget = new.target;
            if (newTarget === bound)
                newTarget = fun;
            switch (arguments.length) {
              case 0:
                return constructContentFunction(fun, newTarget, bound1, bound2);
              case 1:
                return constructContentFunction(fun, newTarget, bound1, bound2, arguments[0]);
              case 2:
                return constructContentFunction(fun, newTarget, bound1, bound2, arguments[0], arguments[1]);
              case 3:
                return constructContentFunction(fun, newTarget, bound1, bound2, arguments[0], arguments[1], arguments[2]);
              case 4:
                return constructContentFunction(fun, newTarget, bound1, bound2, arguments[0], arguments[1], arguments[2], arguments[3]);
              case 5:
                return constructContentFunction(fun, newTarget, bound1, bound2, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
            }
        } else {
            switch (arguments.length) {
              case 0:
                return callContentFunction(fun, thisArg, bound1, bound2);
              case 1:
                return callContentFunction(fun, thisArg, bound1, bound2, arguments[0]);
              case 2:
                return callContentFunction(fun, thisArg, bound1, bound2, arguments[0], arguments[1]);
              case 3:
                return callContentFunction(fun, thisArg, bound1, bound2, arguments[0], arguments[1], arguments[2]);
              case 4:
                return callContentFunction(fun, thisArg, bound1, bound2, arguments[0], arguments[1], arguments[2], arguments[3]);
              case 5:
                return callContentFunction(fun, thisArg, bound1, bound2, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
            }
        }
        if (combiner === null) {
            combiner = function() {
                var callArgsCount = arguments.length;
                var args = std_Array(2 + callArgsCount);
                _DefineDataProperty(args, 0, bound1);
                _DefineDataProperty(args, 1, bound2);
                for (var i = 0; i < callArgsCount; i++)
                    _DefineDataProperty(args, i + 2, arguments[i]);
                return args;
            };
        }
        var args = callFunction(std_Function_apply, combiner, null, arguments);
        if (newTarget === undefined)
            return bind_applyFunctionN(fun, thisArg, args);
        return bind_constructFunctionN(fun, newTarget, args);
    };
}
function bind_bindFunctionN(fun, thisArg, boundArgs) {
    ;;
    var combiner = null;
    return function bound() {
        var newTarget;
        if (_IsConstructing()) {
            newTarget = new.target;
            if (newTarget === bound)
                newTarget = fun;
        }
        if (arguments.length === 0) {
            if (newTarget !== undefined)
                return bind_constructFunctionN(fun, newTarget, boundArgs);
            return bind_applyFunctionN(fun, thisArg, boundArgs);
        }
        if (combiner === null) {
            combiner = function() {
                var boundArgsCount = boundArgs.length;
                var callArgsCount = arguments.length;
                var args = std_Array(boundArgsCount + callArgsCount);
                for (var i = 0; i < boundArgsCount; i++)
                    _DefineDataProperty(args, i, boundArgs[i]);
                for (var i = 0; i < callArgsCount; i++)
                    _DefineDataProperty(args, i + boundArgsCount, arguments[i]);
                return args;
            };
        }
        var args = callFunction(std_Function_apply, combiner, null, arguments);
        if (newTarget !== undefined)
            return bind_constructFunctionN(fun, newTarget, args);
        return bind_applyFunctionN(fun, thisArg, args);
    };
}
function bind_mapArguments() {
    var len = arguments.length;
    var args = std_Array(len);
    for (var i = 0; i < len; i++)
        _DefineDataProperty(args, i, arguments[i]);
    return args;
}
function bind_applyFunctionN(fun, thisArg, args) {
    switch (args.length) {
      case 0:
        return callContentFunction(fun, thisArg);
      case 1:
        return callContentFunction(fun, thisArg, args[0]);
      case 2:
        return callContentFunction(fun, thisArg, args[0], args[1]);
      case 3:
        return callContentFunction(fun, thisArg, args[0], args[1], args[2]);
      case 4:
        return callContentFunction(fun, thisArg, args[0], args[1], args[2], args[3]);
      case 5:
        return callContentFunction(fun, thisArg, args[0], args[1], args[2], args[3], args[4]);
      case 6:
        return callContentFunction(fun, thisArg, args[0], args[1], args[2], args[3], args[4], args[5]);
      case 7:
        return callContentFunction(fun, thisArg, args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
      case 8:
        return callContentFunction(fun, thisArg, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
      case 9:
        return callContentFunction(fun, thisArg, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]);
      default:
        return callFunction(std_Function_apply, fun, thisArg, args);
    }
}
function bind_constructFunctionN(fun, newTarget, args) {
    switch (args.length) {
      case 1:
        return constructContentFunction(fun, newTarget, args[0]);
      case 2:
        return constructContentFunction(fun, newTarget, args[0], args[1]);
      case 3:
        return constructContentFunction(fun, newTarget, args[0], args[1], args[2]);
      case 4:
        return constructContentFunction(fun, newTarget, args[0], args[1], args[2], args[3]);
      case 5:
        return constructContentFunction(fun, newTarget, args[0], args[1], args[2], args[3], args[4]);
      case 6:
        return constructContentFunction(fun, newTarget, args[0], args[1], args[2], args[3], args[4], args[5]);
      case 7:
        return constructContentFunction(fun, newTarget, args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
      case 8:
        return constructContentFunction(fun, newTarget, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
      case 9:
        return constructContentFunction(fun, newTarget, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]);
      case 10:
        return constructContentFunction(fun, newTarget, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);
      case 11:
        return constructContentFunction(fun, newTarget, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10]);
      case 12:
        return constructContentFunction(fun, newTarget, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11]);
      default:
        ;;
        return _ConstructFunction(fun, newTarget, args);
    }
}
function GeneratorNext(val) {
    if (!IsSuspendedGenerator(this)) {
        if (!IsObject(this) || !IsGeneratorObject(this))
            return callFunction(CallGeneratorMethodIfWrapped, this, val, "GeneratorNext");
        if (GeneratorObjectIsClosed(this))
            return { value: undefined, done: true };
        if (GeneratorIsRunning(this))
            ThrowTypeError(34);
    }
    try {
        return resumeGenerator(this, val, "next");
    } catch (e) {
        if (!GeneratorObjectIsClosed(this))
            GeneratorSetClosed(this);
        throw e;
    }
}
function GeneratorThrow(val) {
    if (!IsSuspendedGenerator(this)) {
        if (!IsObject(this) || !IsGeneratorObject(this))
            return callFunction(CallGeneratorMethodIfWrapped, this, val, "GeneratorThrow");
        if (GeneratorObjectIsClosed(this))
            throw val;
        if (GeneratorIsRunning(this))
            ThrowTypeError(34);
    }
    try {
        return resumeGenerator(this, val, "throw");
    } catch (e) {
        if (!GeneratorObjectIsClosed(this))
            GeneratorSetClosed(this);
        throw e;
    }
}
function GeneratorReturn(val) {
    if (!IsSuspendedGenerator(this)) {
        if (!IsObject(this) || !IsGeneratorObject(this))
            return callFunction(CallGeneratorMethodIfWrapped, this, val, "GeneratorReturn");
        if (GeneratorObjectIsClosed(this))
            return { value: val, done: true };
        if (GeneratorIsRunning(this))
            ThrowTypeError(34);
    }
    try {
        var rval = { value: val, done: true };
        return resumeGenerator(this, rval, "return");
    } catch (e) {
        if (!GeneratorObjectIsClosed(this))
            GeneratorSetClosed(this);
        throw e;
    }
}
function InterpretGeneratorResume(gen, val, kind) {
    forceInterpreter();
    if (kind === "next")
       return resumeGenerator(gen, val, "next");
    if (kind === "throw")
       return resumeGenerator(gen, val, "throw");
    ;;
    return resumeGenerator(gen, val, "return");
}
function IteratorIdentity() {
    return this;
}
function MapConstructorInit(iterable) {
    var map = this;
    var adder = map.set;
    if (!IsCallable(adder))
        ThrowTypeError(9, typeof adder);
    for (var nextItem of allowContentIter(iterable)) {
        if (!IsObject(nextItem))
            ThrowTypeError(33, "Map");
        callContentFunction(adder, map, nextItem[0], nextItem[1]);
    }
}
function MapForEach(callbackfn, thisArg = undefined) {
    var M = this;
    if (!IsObject(M) || (M = GuardToMapObject(M)) === null)
        return callFunction(CallMapMethodIfWrapped, this, callbackfn, thisArg, "MapForEach");
    if (!IsCallable(callbackfn))
        ThrowTypeError(9, DecompileArg(0, callbackfn));
    var entries = callFunction(std_Map_iterator, M);
    var mapIterationResultPair = iteratorTemp.mapIterationResultPair;
    if (!mapIterationResultPair) {
        mapIterationResultPair = iteratorTemp.mapIterationResultPair =
            _CreateMapIterationResultPair();
    }
    while (true) {
        var done = _GetNextMapEntryForIterator(entries, mapIterationResultPair);
        if (done)
            break;
        var key = mapIterationResultPair[0];
        var value = mapIterationResultPair[1];
        mapIterationResultPair[0] = null;
        mapIterationResultPair[1] = null;
        callContentFunction(callbackfn, thisArg, value, key, M);
    }
}
function $MapEntries() {
    return callFunction(std_Map_iterator, this);
}
_SetCanonicalName($MapEntries, "entries");
var iteratorTemp = { mapIterationResultPair: null };
function MapIteratorNext() {
    var O = this;
    if (!IsObject(O) || (O = GuardToMapIterator(O)) === null)
        return callFunction(CallMapIteratorMethodIfWrapped, this, "MapIteratorNext");
    var mapIterationResultPair = iteratorTemp.mapIterationResultPair;
    if (!mapIterationResultPair) {
        mapIterationResultPair = iteratorTemp.mapIterationResultPair =
            _CreateMapIterationResultPair();
    }
    var retVal = {value: undefined, done: true};
    var done = _GetNextMapEntryForIterator(O, mapIterationResultPair);
    if (!done) {
        var itemKind = UnsafeGetInt32FromReservedSlot(O, 2);
        var result;
        if (itemKind === 0) {
            result = mapIterationResultPair[0];
        } else if (itemKind === 1) {
            result = mapIterationResultPair[1];
        } else {
            ;;
            result = [mapIterationResultPair[0], mapIterationResultPair[1]];
        }
        mapIterationResultPair[0] = null;
        mapIterationResultPair[1] = null;
        retVal.value = result;
        retVal.done = false;
    }
    return retVal;
}
function $MapSpecies() {
    return this;
}
_SetCanonicalName($MapSpecies, "get [Symbol.species]");
function CallModuleResolveHook(module, specifier, expectedMinimumStatus)
{
    let requestedModule = HostResolveImportedModule(module, specifier);
    if (requestedModule.status < expectedMinimumStatus)
        ThrowInternalError(543);
    return requestedModule;
}
function ModuleGetExportedNames(exportStarSet = [])
{
    if (!IsObject(this) || !IsModule(this)) {
        return callFunction(CallModuleMethodIfWrapped, this, exportStarSet,
                            "ModuleGetExportedNames");
    }
    let module = this;
    if (callFunction(ArrayIncludes, exportStarSet, module))
        return [];
    _DefineDataProperty(exportStarSet, exportStarSet.length, module);
    let exportedNames = [];
    let namesCount = 0;
    let localExportEntries = module.localExportEntries;
    for (let i = 0; i < localExportEntries.length; i++) {
        let e = localExportEntries[i];
        _DefineDataProperty(exportedNames, namesCount++, e.exportName);
    }
    let indirectExportEntries = module.indirectExportEntries;
    for (let i = 0; i < indirectExportEntries.length; i++) {
        let e = indirectExportEntries[i];
        _DefineDataProperty(exportedNames, namesCount++, e.exportName);
    }
    let starExportEntries = module.starExportEntries;
    for (let i = 0; i < starExportEntries.length; i++) {
        let e = starExportEntries[i];
        let requestedModule = CallModuleResolveHook(module, e.moduleRequest,
                                                    0);
        let starNames = callFunction(requestedModule.getExportedNames, requestedModule,
                                     exportStarSet);
        for (let j = 0; j < starNames.length; j++) {
            let n = starNames[j];
            if (n !== "default" && !callFunction(ArrayIncludes, exportedNames, n))
                _DefineDataProperty(exportedNames, namesCount++, n);
        }
    }
    return exportedNames;
}
function ModuleSetStatus(module, newStatus)
{
    ;;
    ;;
    UnsafeSetReservedSlot(module, 3, newStatus);
}
function ModuleResolveExport(exportName, resolveSet = [])
{
    if (!IsObject(this) || !IsModule(this)) {
        return callFunction(CallModuleMethodIfWrapped, this, exportName, resolveSet,
                            "ModuleResolveExport");
    }
    let module = this;
    for (let i = 0; i < resolveSet.length; i++) {
        let r = resolveSet[i];
        if (r.module === module && r.exportName === exportName) {
            return null;
        }
    }
    _DefineDataProperty(resolveSet, resolveSet.length, {module, exportName});
    let localExportEntries = module.localExportEntries;
    for (let i = 0; i < localExportEntries.length; i++) {
        let e = localExportEntries[i];
        if (exportName === e.exportName)
            return {module, bindingName: e.localName};
    }
    let indirectExportEntries = module.indirectExportEntries;
    for (let i = 0; i < indirectExportEntries.length; i++) {
        let e = indirectExportEntries[i];
        if (exportName === e.exportName) {
            let importedModule = CallModuleResolveHook(module, e.moduleRequest,
                                                       0);
            return callFunction(importedModule.resolveExport, importedModule, e.importName,
                                resolveSet);
        }
    }
    if (exportName === "default") {
        return null;
    }
    let starResolution = null;
    let starExportEntries = module.starExportEntries;
    for (let i = 0; i < starExportEntries.length; i++) {
        let e = starExportEntries[i];
        let importedModule = CallModuleResolveHook(module, e.moduleRequest,
                                                   0);
        let resolution = callFunction(importedModule.resolveExport, importedModule, exportName,
                                      resolveSet);
        if (resolution === "ambiguous")
            return resolution;
        if (resolution !== null) {
            if (starResolution === null) {
                starResolution = resolution;
            } else {
                if (resolution.module !== starResolution.module ||
                    resolution.bindingName !== starResolution.bindingName)
                {
                    return "ambiguous";
                }
            }
        }
    }
    return starResolution;
}
function IsResolvedBinding(resolution)
{
    ;;
    return typeof resolution === "object" && resolution !== null;
}
function GetModuleNamespace(module)
{
    ;;
    ;;
    let namespace = module.namespace;
    if (typeof namespace === "undefined") {
        let exportedNames = callFunction(module.getExportedNames, module);
        let unambiguousNames = [];
        for (let i = 0; i < exportedNames.length; i++) {
            let name = exportedNames[i];
            let resolution = callFunction(module.resolveExport, module, name);
            if (IsResolvedBinding(resolution))
                _DefineDataProperty(unambiguousNames, unambiguousNames.length, name);
        }
        namespace = ModuleNamespaceCreate(module, unambiguousNames);
    }
    return namespace;
}
function ModuleNamespaceCreate(module, exports)
{
    callFunction(ArraySort, exports);
    let ns = NewModuleNamespace(module, exports);
    for (let i = 0; i < exports.length; i++) {
        let name = exports[i];
        let binding = callFunction(module.resolveExport, module, name);
        ;;
        AddModuleNamespaceBinding(ns, name, binding.module, binding.bindingName);
    }
    return ns;
}
function GetModuleEnvironment(module)
{
    ;;
    ;;
    let env = UnsafeGetReservedSlot(module, 1);
    ;;
    return env;
}
function CountArrayValues(array, value)
{
    let count = 0;
    for (let i = 0; i < array.length; i++) {
        if (array[i] === value)
            count++;
    }
    return count;
}
function ArrayContains(array, value)
{
    for (let i = 0; i < array.length; i++) {
        if (array[i] === value)
            return true;
    }
    return false;
}
function HandleModuleInstantiationFailure(module)
{
    ModuleSetStatus(module, 0);
    UnsafeSetReservedSlot(module, 14, undefined);
    UnsafeSetReservedSlot(module, 15, undefined);
}
function ModuleInstantiate()
{
    if (!IsObject(this) || !IsModule(this))
        return callFunction(CallModuleMethodIfWrapped, this, "ModuleInstantiate");
    let module = this;
    if (module.status === 1 ||
        module.status === 3)
    {
        ThrowInternalError(543);
    }
    let stack = [];
    try {
        InnerModuleInstantiation(module, stack, 0);
    } catch (error) {
        for (let i = 0; i < stack.length; i++) {
            let m = stack[i];
            if (m.status === 1) {
                HandleModuleInstantiationFailure(m);
            }
        }
        if (stack.length === 0 && module.status === 1) {
            HandleModuleInstantiationFailure(module);
        }
        ;;
        throw error;
    }
    ;;
    ;;
    return undefined;
}
function InnerModuleInstantiation(module, stack, index)
{
    if (module.status === 1 ||
        module.status === 2 ||
        module.status === 4 ||
        module.status === 5)
    {
        return index;
    }
    if (module.status !== 0)
        ThrowInternalError(543);
    ModuleSetStatus(module, 1);
    UnsafeSetReservedSlot(module, 14, index);
    UnsafeSetReservedSlot(module, 15, index);
    index++;
    _DefineDataProperty(stack, stack.length, module);
    let requestedModules = module.requestedModules;
    for (let i = 0; i < requestedModules.length; i++) {
        let required = requestedModules[i].moduleSpecifier;
        let requiredModule = CallModuleResolveHook(module, required, 0);
        index = InnerModuleInstantiation(requiredModule, stack, index);
        ;;
        ;;
        ;;
        ;;
        if (requiredModule.status === 1) {
            UnsafeSetReservedSlot(module, 15,
                                  std_Math_min(module.dfsAncestorIndex,
                                               requiredModule.dfsAncestorIndex));
        }
    }
    ModuleDeclarationEnvironmentSetup(module);
    ;;
    ;;
    if (module.dfsAncestorIndex === module.dfsIndex) {
        let requiredModule;
        do {
            requiredModule = callFunction(std_Array_pop, stack);
            ModuleSetStatus(requiredModule, 2);
        } while (requiredModule !== module);
    }
    return index;
}
function ModuleDeclarationEnvironmentSetup(module)
{
    let indirectExportEntries = module.indirectExportEntries;
    for (let i = 0; i < indirectExportEntries.length; i++) {
        let e = indirectExportEntries[i];
        let resolution = callFunction(module.resolveExport, module, e.exportName);
        if (!IsResolvedBinding(resolution)) {
            ThrowResolutionError(module, resolution, "indirectExport", e.exportName,
                                 e.lineNumber, e.columnNumber);
        }
    }
    let env = GetModuleEnvironment(module);
    let importEntries = module.importEntries;
    for (let i = 0; i < importEntries.length; i++) {
        let imp = importEntries[i];
        let importedModule = CallModuleResolveHook(module, imp.moduleRequest,
                                                   1);
        if (imp.importName === "*") {
            let namespace = GetModuleNamespace(importedModule);
            CreateNamespaceBinding(env, imp.localName, namespace);
        } else {
            let resolution = callFunction(importedModule.resolveExport, importedModule,
                                          imp.importName);
            if (!IsResolvedBinding(resolution)) {
                ThrowResolutionError(module, resolution, "import", imp.importName,
                                     imp.lineNumber, imp.columnNumber);
            }
            CreateImportBinding(env, imp.localName, resolution.module, resolution.bindingName);
        }
    }
    InstantiateModuleFunctionDeclarations(module);
}
function ThrowResolutionError(module, resolution, kind, name, line, column)
{
    ;;
    ;;
    ;;
    let ambiguous = resolution === "ambiguous";
    let errorNumber;
    if (kind === "import")
        errorNumber = ambiguous ? 540 : 539;
    else
        errorNumber = ambiguous ? 538 : 537;
    let message = GetErrorMessage(errorNumber) + ": " + name;
    let error = CreateModuleSyntaxError(module, line, column, message);
    throw error;
}
function GetModuleEvaluationError(module)
{
    ;;
    ;;
    return UnsafeGetReservedSlot(module, 4);
}
function RecordModuleEvaluationError(module, error)
{
    ;;
    if (module.status === 5) {
        return;
    }
    ModuleSetStatus(module, 5);
    UnsafeSetReservedSlot(module, 4, error);
}
function ModuleEvaluate()
{
    if (!IsObject(this) || !IsModule(this))
        return callFunction(CallModuleMethodIfWrapped, this, "ModuleEvaluate");
    let module = this;
    if (module.status !== 2 &&
        module.status !== 4 &&
        module.status !== 5)
    {
        ThrowInternalError(543);
    }
    let stack = [];
    try {
        InnerModuleEvaluation(module, stack, 0);
    } catch (error) {
        for (let i = 0; i < stack.length; i++) {
            let m = stack[i];
            ;;
            RecordModuleEvaluationError(m, error);
        }
        if (stack.length === 0)
            RecordModuleEvaluationError(module, error);
        ;;
        throw error;
    }
    ;;
    ;;
    return undefined;
}
function InnerModuleEvaluation(module, stack, index)
{
    if (module.status === 5)
        throw GetModuleEvaluationError(module);
    if (module.status === 4)
        return index;
    if (module.status === 3)
        return index;
    ;;
    ModuleSetStatus(module, 3);
    UnsafeSetReservedSlot(module, 14, index);
    UnsafeSetReservedSlot(module, 15, index);
    index++;
    _DefineDataProperty(stack, stack.length, module);
    let requestedModules = module.requestedModules;
    for (let i = 0; i < requestedModules.length; i++) {
        let required = requestedModules[i].moduleSpecifier;
        let requiredModule =
            CallModuleResolveHook(module, required, 2);
        index = InnerModuleEvaluation(requiredModule, stack, index);
        ;;
        ;;
        ;;
        ;;
        if (requiredModule.status === 3) {
            UnsafeSetReservedSlot(module, 15,
                                  std_Math_min(module.dfsAncestorIndex,
                                               requiredModule.dfsAncestorIndex));
        }
    }
    ExecuteModule(module);
    ;;
    ;;
    if (module.dfsAncestorIndex === module.dfsIndex) {
        let requiredModule;
        do {
            requiredModule = callFunction(std_Array_pop, stack);
            ModuleSetStatus(requiredModule, 4);
        } while (requiredModule !== module);
    }
    return index;
}
function Number_isFinite(num) {
    if (typeof num !== "number")
        return false;
    return num - num === 0;
}
function Number_isNaN(num) {
    if (typeof num !== "number")
        return false;
    return num !== num;
}
function Number_isInteger(number) {
    if (typeof number !== "number")
        return false;
    if (number === -(2 ** 31))
        return true;
    var absNumber = std_Math_abs(number);
    var integer = std_Math_floor(absNumber);
    if (absNumber - integer !== 0)
        return false;
    return true;
}
function Number_isSafeInteger(number) {
    if (typeof number !== "number")
        return false;
    if (number === -(2 ** 31))
        return true;
    var absNumber = std_Math_abs(number);
    var integer = std_Math_floor(absNumber);
    if (absNumber - integer !== 0)
        return false;
    if (integer <= (2 ** 53) - 1)
        return true;
    return false;
}
function Global_isNaN(number) {
    return Number_isNaN(ToNumber(number));
}
function Global_isFinite(number) {
    return Number_isFinite(ToNumber(number));
}
function ObjectGetOwnPropertyDescriptors(O) {
    var obj = ToObject(O);
    var keys = std_Reflect_ownKeys(obj);
    var descriptors = {};
    for (var index = 0, len = keys.length; index < len; index++) {
        var key = keys[index];
        var desc = ObjectGetOwnPropertyDescriptor(obj, key);
        if (typeof desc !== "undefined")
            _DefineDataProperty(descriptors, key, desc);
    }
    return descriptors;
}
function ObjectGetPrototypeOf(obj) {
    return std_Reflect_getPrototypeOf(ToObject(obj));
}
function ObjectIsExtensible(obj) {
    return IsObject(obj) && std_Reflect_isExtensible(obj);
}
function Object_toLocaleString() {
    var O = this;
    return callContentFunction(O.toString, O);
}
function Object_valueOf() {
    return ToObject(this);
}
function Object_hasOwnProperty(V) {
    return hasOwn(V, this);
}
function $ObjectProtoGetter() {
    return std_Reflect_getPrototypeOf(ToObject(this));
}
_SetCanonicalName($ObjectProtoGetter, "get __proto__");
function $ObjectProtoSetter(proto) {
    return callFunction(std_Object_setProto, this, proto);
}
_SetCanonicalName($ObjectProtoSetter, "set __proto__");
function ObjectDefineSetter(name, setter) {
    var object = ToObject(this);
    if (!IsCallable(setter))
        ThrowTypeError(25, "setter");
    var key = (typeof name !== "string" && typeof name !== "number" && typeof name !== "symbol" ? ToPropertyKey(name) : name);
    _DefineProperty(object, key, 0x200 | 0x01 | 0x02,
                    null, setter, true);
}
function ObjectDefineGetter(name, getter) {
    var object = ToObject(this);
    if (!IsCallable(getter))
        ThrowTypeError(25, "getter");
    var key = (typeof name !== "string" && typeof name !== "number" && typeof name !== "symbol" ? ToPropertyKey(name) : name);
    _DefineProperty(object, key, 0x200 | 0x01 | 0x02,
                    getter, null, true);
}
function ObjectLookupSetter(name) {
    var object = ToObject(this);
    var key = (typeof name !== "string" && typeof name !== "number" && typeof name !== "symbol" ? ToPropertyKey(name) : name);
    do {
        var desc = GetOwnPropertyDescriptorToArray(object, key);
        if (desc) {
            if (desc[0] & 0x200)
                return desc[2];
            return undefined;
        }
        object = std_Reflect_getPrototypeOf(object);
    } while (object !== null);
}
function ObjectLookupGetter(name) {
    var object = ToObject(this);
    var key = (typeof name !== "string" && typeof name !== "number" && typeof name !== "symbol" ? ToPropertyKey(name) : name);
    do {
        var desc = GetOwnPropertyDescriptorToArray(object, key);
        if (desc) {
            if (desc[0] & 0x200)
                return desc[1];
            return undefined;
        }
        object = std_Reflect_getPrototypeOf(object);
    } while (object !== null);
}
function ObjectGetOwnPropertyDescriptor(obj, propertyKey) {
    var desc = GetOwnPropertyDescriptorToArray(obj, propertyKey);
    if (!desc)
        return undefined;
    var attrsAndKind = desc[0];
    if (attrsAndKind & 0x100) {
        return {
            value: desc[1],
            writable: !!(attrsAndKind & 0x04),
            enumerable: !!(attrsAndKind & 0x01),
            configurable: !!(attrsAndKind & 0x02),
        };
    }
    ;;
    return {
        get: desc[1],
        set: desc[2],
        enumerable: !!(attrsAndKind & 0x01),
        configurable: !!(attrsAndKind & 0x02),
    };
}
function ObjectOrReflectDefineProperty(obj, propertyKey, attributes, strict) {
    if (!IsObject(obj))
        ThrowTypeError(43, DecompileArg(0, obj));
    propertyKey = (typeof propertyKey !== "string" && typeof propertyKey !== "number" && typeof propertyKey !== "symbol" ? ToPropertyKey(propertyKey) : propertyKey);
    if (!IsObject(attributes))
        ThrowArgTypeNotObject(0, attributes);
    var attrs = 0, hasValue = false;
    var value, getter = null, setter = null;
    if ("enumerable" in attributes)
        attrs |= attributes.enumerable ? 0x01 : 0x08;
    if ("configurable" in attributes)
        attrs |= attributes.configurable ? 0x02 : 0x10;
    if ("value" in attributes) {
        attrs |= 0x100;
        value = attributes.value;
        hasValue = true;
    }
    if ("writable" in attributes) {
        attrs |= 0x100;
        attrs |= attributes.writable ? 0x04 : 0x20;
    }
    if ("get" in attributes) {
        attrs |= 0x200;
        getter = attributes.get;
        if (!IsCallable(getter) && getter !== undefined)
            ThrowTypeError(57, "get");
    }
    if ("set" in attributes) {
        attrs |= 0x200;
        setter = attributes.set;
        if (!IsCallable(setter) && setter !== undefined)
            ThrowTypeError(57, "set");
    }
    if (attrs & 0x200) {
        if (attrs & 0x100)
            ThrowTypeError(51);
        return _DefineProperty(obj, propertyKey, attrs, getter, setter, strict);
    }
    if (hasValue) {
        if (strict) {
            if ((attrs & (0x01 | 0x02 | 0x04)) ===
                (0x01 | 0x02 | 0x04))
            {
                _DefineDataProperty(obj, propertyKey, value);
                return true;
            }
        }
        return _DefineProperty(obj, propertyKey, attrs, value, null, strict);
    }
    return _DefineProperty(obj, propertyKey, attrs, undefined, undefined, strict);
}
function ObjectDefineProperty(obj, propertyKey, attributes) {
    if (!ObjectOrReflectDefineProperty(obj, propertyKey, attributes, true)) {
        return null;
    }
    return obj;
}
function ObjectFromEntries(iter) {
    const obj = {};
    for (const pair of allowContentIter(iter)) {
        if (!IsObject(pair))
            ThrowTypeError(33, "Object.fromEntries");
        _DefineDataProperty(obj, pair[0], pair[1]);
    }
    return obj;
}
function Promise_finally(onFinally) {
    var promise = this;
    if (!IsObject(promise))
        ThrowTypeError(3, "Promise", "finally", "value");
    var C = SpeciesConstructor(promise, GetBuiltinConstructor("Promise"));
    ;;
    var thenFinally, catchFinally;
    if (!IsCallable(onFinally)) {
        thenFinally = onFinally;
        catchFinally = onFinally;
    } else {
        (thenFinally) = function(value) {
            var result = onFinally();
            var promise = PromiseResolve(C, result);
            return callContentFunction(promise.then, promise, function() { return value; });
        };
        (catchFinally) = function(reason) {
            var result = onFinally();
            var promise = PromiseResolve(C, result);
            return callContentFunction(promise.then, promise, function() { throw reason; });
        };
    }
    return callContentFunction(promise.then, promise, thenFinally, catchFinally);
}
function CreateListFromArrayLikeForArgs(obj) {
    ;;
    var len = ToLength(obj.length);
    if (len > (500 * 1000))
        ThrowRangeError(105);
    var list = std_Array(len);
    for (var i = 0; i < len; i++)
        _DefineDataProperty(list, i, obj[i]);
    return list;
}
function Reflect_apply(target, thisArgument, argumentsList) {
    if (!IsCallable(target))
        ThrowTypeError(9, DecompileArg(0, target));
    if (!IsObject(argumentsList)) {
        ThrowTypeError(44, "`argumentsList`", "Reflect.apply",
                       ToSource(argumentsList));
    }
    return callFunction(std_Function_apply, target, thisArgument, argumentsList);
}
function Reflect_construct(target, argumentsList) {
    if (!IsConstructor(target))
        ThrowTypeError(10, DecompileArg(0, target));
    var newTarget;
    if (arguments.length > 2) {
        newTarget = arguments[2];
        if (!IsConstructor(newTarget))
            ThrowTypeError(10, DecompileArg(2, newTarget));
    } else {
        newTarget = target;
    }
    if (!IsObject(argumentsList)) {
        ThrowTypeError(44, "`argumentsList`", "Reflect.construct",
                       ToSource(argumentsList));
    }
    var args = (IsPackedArray(argumentsList) && argumentsList.length <= (500 * 1000))
               ? argumentsList
               : CreateListFromArrayLikeForArgs(argumentsList);
    switch (args.length) {
      case 0:
        return constructContentFunction(target, newTarget);
      case 1:
        return constructContentFunction(target, newTarget, args[0]);
      case 2:
        return constructContentFunction(target, newTarget, args[0], args[1]);
      case 3:
        return constructContentFunction(target, newTarget, args[0], args[1], args[2]);
      case 4:
        return constructContentFunction(target, newTarget, args[0], args[1], args[2], args[3]);
      case 5:
        return constructContentFunction(target, newTarget, args[0], args[1], args[2], args[3], args[4]);
      case 6:
        return constructContentFunction(target, newTarget, args[0], args[1], args[2], args[3], args[4], args[5]);
      case 7:
        return constructContentFunction(target, newTarget, args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
      case 8:
        return constructContentFunction(target, newTarget, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
      case 9:
        return constructContentFunction(target, newTarget, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]);
      case 10:
        return constructContentFunction(target, newTarget, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);
      case 11:
        return constructContentFunction(target, newTarget, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10]);
      case 12:
        return constructContentFunction(target, newTarget, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11]);
      default:
        return _ConstructFunction(target, newTarget, args);
    }
}
function Reflect_defineProperty(obj, propertyKey, attributes) {
    return ObjectOrReflectDefineProperty(obj, propertyKey, attributes, false);
}
function Reflect_getOwnPropertyDescriptor(target, propertyKey) {
    if (!IsObject(target))
        ThrowTypeError(43, DecompileArg(0, target));
    return ObjectGetOwnPropertyDescriptor(target, propertyKey);
}
function Reflect_has(target, propertyKey) {
    if (!IsObject(target)) {
        ThrowTypeError(44, "`target`", "Reflect.has",
                       ToSource(target));
    }
    return propertyKey in target;
}
function Reflect_get(target, propertyKey) {
    if (!IsObject(target)) {
        ThrowTypeError(44, "`target`", "Reflect.get",
                       ToSource(target));
    }
    if (arguments.length > 2) {
        return getPropertySuper(target, propertyKey, arguments[2]);
    }
    return target[propertyKey];
}
function $RegExpFlagsGetter() {
    var R = this;
    if (!IsObject(R))
        ThrowTypeError(43, R === null ? "null" : typeof R);
    var result = "";
    if (R.global)
        result += "g";
    if (R.ignoreCase)
        result += "i";
    if (R.multiline)
        result += "m";
    if (R.unicode)
         result += "u";
    if (R.sticky)
        result += "y";
    return result;
}
_SetCanonicalName($RegExpFlagsGetter, "get flags");
function $RegExpToString()
{
    var R = this;
    if (!IsObject(R))
        ThrowTypeError(43, R === null ? "null" : typeof R);
    var pattern = ToString(R.source);
    var flags = ToString(R.flags);
    return "/" + pattern + "/" + flags;
}
_SetCanonicalName($RegExpToString, "toString");
function AdvanceStringIndex(S, index) {
    ;;
    ;;
    var length = S.length;
    if (index + 1 >= length)
        return index + 1;
    var first = callFunction(std_String_charCodeAt, S, index);
    if (first < 0xD800 || first > 0xDBFF)
        return index + 1;
    var second = callFunction(std_String_charCodeAt, S, index + 1);
    if (second < 0xDC00 || second > 0xDFFF)
        return index + 1;
    return index + 2;
}
function RegExpMatch(string) {
    var rx = this;
    if (!IsObject(rx))
        ThrowTypeError(43, rx === null ? "null" : typeof rx);
    var S = ToString(string);
    if (IsRegExpMethodOptimizable(rx)) {
        var flags = UnsafeGetInt32FromReservedSlot(rx, 2);
        var global = !!(flags & 0x02);
        if (global) {
            var fullUnicode = !!(flags & 0x10);
            return RegExpGlobalMatchOpt(rx, S, fullUnicode);
        }
        return RegExpBuiltinExec(rx, S, false);
    }
    return RegExpMatchSlowPath(rx, S);
}
function RegExpMatchSlowPath(rx, S) {
    if (!rx.global)
        return RegExpExec(rx, S, false);
    var fullUnicode = !!rx.unicode;
    rx.lastIndex = 0;
    var A = [];
    var n = 0;
    while (true) {
        var result = RegExpExec(rx, S, false);
        if (result === null)
          return (n === 0) ? null : A;
        var matchStr = ToString(result[0]);
        _DefineDataProperty(A, n, matchStr);
        if (matchStr === "") {
            var lastIndex = ToLength(rx.lastIndex);
            rx.lastIndex = fullUnicode ? AdvanceStringIndex(S, lastIndex) : lastIndex + 1;
        }
        n++;
    }
}
function RegExpGlobalMatchOpt(rx, S, fullUnicode) {
    var lastIndex = 0;
    rx.lastIndex = 0;
    var A = [];
    var n = 0;
    var lengthS = S.length;
    while (true) {
        var result = RegExpMatcher(rx, S, lastIndex);
        if (result === null)
            return (n === 0) ? null : A;
        lastIndex = result.index + result[0].length;
        var matchStr = result[0];
        _DefineDataProperty(A, n, matchStr);
        if (matchStr === "") {
            lastIndex = fullUnicode ? AdvanceStringIndex(S, lastIndex) : lastIndex + 1;
            if (lastIndex > lengthS)
                return A;
        }
        n++;
    }
}
function IsRegExpMethodOptimizable(rx) {
    if (!IsRegExpObject(rx))
        return false;
    var RegExpProto = GetBuiltinPrototype("RegExp");
    return RegExpPrototypeOptimizable(RegExpProto) &&
           RegExpInstanceOptimizable(rx, RegExpProto) &&
           RegExpProto.exec === RegExp_prototype_Exec;
}
function RegExpReplace(string, replaceValue) {
    var rx = this;
    if (!IsObject(rx))
        ThrowTypeError(43, rx === null ? "null" : typeof rx);
    var S = ToString(string);
    var lengthS = S.length;
    var functionalReplace = IsCallable(replaceValue);
    var firstDollarIndex = -1;
    if (!functionalReplace) {
        replaceValue = ToString(replaceValue);
        if (replaceValue.length > 1)
            firstDollarIndex = GetFirstDollarIndex(replaceValue);
    }
    if (IsRegExpMethodOptimizable(rx)) {
        var flags = UnsafeGetInt32FromReservedSlot(rx, 2);
        var global = !!(flags & 0x02);
        if (global) {
            if (functionalReplace) {
                if (lengthS > 5000) {
                    var elemBase = GetElemBaseForLambda(replaceValue);
                    if (IsObject(elemBase)) {
                        return RegExpGlobalReplaceOptElemBase(rx, S, lengthS, replaceValue, flags,
                                                              elemBase);
                    }
                }
                return RegExpGlobalReplaceOptFunc(rx, S, lengthS, replaceValue, flags);
            }
            if (firstDollarIndex !== -1) {
                return RegExpGlobalReplaceOptSubst(rx, S, lengthS, replaceValue, flags,
                                                   firstDollarIndex);
            }
            if (lengthS < 0x7fff)
                return RegExpGlobalReplaceShortOpt(rx, S, lengthS, replaceValue, flags);
            return RegExpGlobalReplaceOpt(rx, S, lengthS, replaceValue, flags);
        }
        if (functionalReplace)
            return RegExpLocalReplaceOptFunc(rx, S, lengthS, replaceValue);
        if (firstDollarIndex !== -1)
            return RegExpLocalReplaceOptSubst(rx, S, lengthS, replaceValue, firstDollarIndex);
        if (lengthS < 0x7fff)
            return RegExpLocalReplaceOptShort(rx, S, lengthS, replaceValue);
        return RegExpLocalReplaceOpt(rx, S, lengthS, replaceValue);
    }
    return RegExpReplaceSlowPath(rx, S, lengthS, replaceValue,
                                 functionalReplace, firstDollarIndex);
}
function RegExpReplaceSlowPath(rx, S, lengthS, replaceValue,
                               functionalReplace, firstDollarIndex)
{
    var global = !!rx.global;
    var fullUnicode = false;
    if (global) {
        fullUnicode = !!rx.unicode;
        rx.lastIndex = 0;
    }
    var results = [];
    var nResults = 0;
    while (true) {
        var result = RegExpExec(rx, S, false);
        if (result === null)
            break;
        _DefineDataProperty(results, nResults++, result);
        if (!global)
            break;
        var matchStr = ToString(result[0]);
        if (matchStr === "") {
            var lastIndex = ToLength(rx.lastIndex);
            rx.lastIndex = fullUnicode ? AdvanceStringIndex(S, lastIndex) : lastIndex + 1;
        }
    }
    var accumulatedResult = "";
    var nextSourcePosition = 0;
    for (var i = 0; i < nResults; i++) {
        result = results[i];
        var nCaptures = std_Math_max(ToLength(result.length) - 1, 0);
        var matched = ToString(result[0]);
        var matchLength = matched.length;
        var position = std_Math_max(std_Math_min(ToInteger(result.index), lengthS), 0);
        var n, capN, replacement;
        if (functionalReplace || firstDollarIndex !== -1) {
            replacement = RegExpGetComplexReplacement(result, matched, S, position,
                                                      nCaptures, replaceValue,
                                                      functionalReplace, firstDollarIndex);
        } else {
            for (n = 1; n <= nCaptures; n++) {
                capN = result[n];
                if (capN !== undefined)
                    ToString(capN);
            }
            var namedCaptures = result.groups;
            if (namedCaptures !== undefined)
                ToObject(namedCaptures);
            replacement = replaceValue;
        }
        if (position >= nextSourcePosition) {
            accumulatedResult += Substring(S, nextSourcePosition,
                                           position - nextSourcePosition) + replacement;
            nextSourcePosition = position + matchLength;
        }
    }
    if (nextSourcePosition >= lengthS)
        return accumulatedResult;
    return accumulatedResult + Substring(S, nextSourcePosition, lengthS - nextSourcePosition);
}
function RegExpGetComplexReplacement(result, matched, S, position,
                                     nCaptures, replaceValue,
                                     functionalReplace, firstDollarIndex)
{
    var captures = [];
    var capturesLength = 0;
    _DefineDataProperty(captures, capturesLength++, matched);
    for (var n = 1; n <= nCaptures; n++) {
        var capN = result[n];
        if (capN !== undefined)
            capN = ToString(capN);
        _DefineDataProperty(captures, capturesLength++, capN);
    }
    var namedCaptures = result.groups;
    if (functionalReplace) {
        if (namedCaptures === undefined) {
            switch (nCaptures) {
              case 0:
                return ToString(replaceValue(captures[0], position, S));
              case 1:
                return ToString(replaceValue(captures[0], captures[1], position, S));
              case 2:
                return ToString(replaceValue(captures[0], captures[1], captures[2], position, S));
              case 3:
                return ToString(replaceValue(captures[0], captures[1], captures[2], captures[3], position, S));
              case 4:
                return ToString(replaceValue(captures[0], captures[1], captures[2], captures[3], captures[4], position, S));
            }
        }
        _DefineDataProperty(captures, capturesLength++, position);
        _DefineDataProperty(captures, capturesLength++, S);
        if (namedCaptures !== undefined) {
            _DefineDataProperty(captures, capturesLength++, namedCaptures);
        }
        return ToString(callFunction(std_Function_apply, replaceValue, undefined, captures));
    }
    if (namedCaptures !== undefined) {
        namedCaptures = ToObject(namedCaptures);
    }
    return RegExpGetSubstitution(captures, S, position, replaceValue, firstDollarIndex,
                                 namedCaptures);
}
function RegExpGetFunctionalReplacement(result, S, position, replaceValue) {
    ;;
    var nCaptures = result.length - 1;
    var namedCaptures = result.groups;
    if (namedCaptures === undefined) {
        switch (nCaptures) {
          case 0:
            return ToString(replaceValue(result[0], position, S));
          case 1:
            return ToString(replaceValue(result[0], result[1], position, S));
          case 2:
            return ToString(replaceValue(result[0], result[1], result[2], position, S));
          case 3:
            return ToString(replaceValue(result[0], result[1], result[2], result[3], position, S));
          case 4:
            return ToString(replaceValue(result[0], result[1], result[2], result[3], result[4], position, S));
        }
    }
    var captures = [];
    for (var n = 0; n <= nCaptures; n++) {
        ;;
        _DefineDataProperty(captures, n, result[n]);
    }
    _DefineDataProperty(captures, nCaptures + 1, position);
    _DefineDataProperty(captures, nCaptures + 2, S);
    if (namedCaptures !== undefined) {
        _DefineDataProperty(captures, nCaptures + 3, namedCaptures);
    }
    return ToString(callFunction(std_Function_apply, replaceValue, undefined, captures));
}
function RegExpGlobalReplaceShortOpt(rx, S, lengthS, replaceValue, flags)
{
    var fullUnicode = !!(flags & 0x10);
    var lastIndex = 0;
    rx.lastIndex = 0;
    var accumulatedResult = "";
    var nextSourcePosition = 0;
    while (true) {
        var result = RegExpSearcher(rx, S, lastIndex);
        if (result === -1)
            break;
        var position = result & 0x7fff;
        lastIndex = (result >> 15) & 0x7fff;
        accumulatedResult += Substring(S, nextSourcePosition,
                                       position - nextSourcePosition) + replaceValue;
        nextSourcePosition = lastIndex;
        if (lastIndex === position) {
            lastIndex = fullUnicode ? AdvanceStringIndex(S, lastIndex) : lastIndex + 1;
            if (lastIndex > lengthS)
                break;
        }
    }
    if (nextSourcePosition >= lengthS)
        return accumulatedResult;
    return accumulatedResult + Substring(S, nextSourcePosition, lengthS - nextSourcePosition);
}
function RegExpGlobalReplaceOpt(rx, S, lengthS, replaceValue, flags
                  )
{
    var fullUnicode = !!(flags & 0x10);
    var lastIndex = 0;
    rx.lastIndex = 0;
    var accumulatedResult = "";
    var nextSourcePosition = 0;
    while (true) {
        var result = RegExpMatcher(rx, S, lastIndex);
        if (result === null)
            break;
        ;;
        var matched = result[0];
        var matchLength = matched.length | 0;
        var position = result.index | 0;
        lastIndex = position + matchLength;
        var replacement;
        replacement = replaceValue;
        accumulatedResult += Substring(S, nextSourcePosition,
                                       position - nextSourcePosition) + replacement;
        nextSourcePosition = lastIndex;
        if (matchLength === 0) {
            lastIndex = fullUnicode ? AdvanceStringIndex(S, lastIndex) : lastIndex + 1;
            if (lastIndex > lengthS)
                break;
            lastIndex |= 0;
        }
    }
    if (nextSourcePosition >= lengthS)
        return accumulatedResult;
    return accumulatedResult + Substring(S, nextSourcePosition, lengthS - nextSourcePosition);
}
function RegExpGlobalReplaceOptFunc(rx, S, lengthS, replaceValue, flags
                  )
{
    var fullUnicode = !!(flags & 0x10);
    var lastIndex = 0;
    rx.lastIndex = 0;
    var originalSource = UnsafeGetStringFromReservedSlot(rx, 1);
    var originalFlags = flags;
    var accumulatedResult = "";
    var nextSourcePosition = 0;
    while (true) {
        var result = RegExpMatcher(rx, S, lastIndex);
        if (result === null)
            break;
        ;;
        var matched = result[0];
        var matchLength = matched.length | 0;
        var position = result.index | 0;
        lastIndex = position + matchLength;
        var replacement;
        replacement = RegExpGetFunctionalReplacement(result, S, position, replaceValue);
        accumulatedResult += Substring(S, nextSourcePosition,
                                       position - nextSourcePosition) + replacement;
        nextSourcePosition = lastIndex;
        if (matchLength === 0) {
            lastIndex = fullUnicode ? AdvanceStringIndex(S, lastIndex) : lastIndex + 1;
            if (lastIndex > lengthS)
                break;
            lastIndex |= 0;
        }
        if (UnsafeGetStringFromReservedSlot(rx, 1) !== originalSource ||
            UnsafeGetInt32FromReservedSlot(rx, 2) !== originalFlags)
        {
            rx = regexp_construct_raw_flags(originalSource, originalFlags);
        }
    }
    if (nextSourcePosition >= lengthS)
        return accumulatedResult;
    return accumulatedResult + Substring(S, nextSourcePosition, lengthS - nextSourcePosition);
}
function RegExpGlobalReplaceOptElemBase(rx, S, lengthS, replaceValue, flags
                   , elemBase
                  )
{
    var fullUnicode = !!(flags & 0x10);
    var lastIndex = 0;
    rx.lastIndex = 0;
    var originalSource = UnsafeGetStringFromReservedSlot(rx, 1);
    var originalFlags = flags;
    var accumulatedResult = "";
    var nextSourcePosition = 0;
    while (true) {
        var result = RegExpMatcher(rx, S, lastIndex);
        if (result === null)
            break;
        ;;
        var matched = result[0];
        var matchLength = matched.length | 0;
        var position = result.index | 0;
        lastIndex = position + matchLength;
        var replacement;
        if (IsObject(elemBase)) {
            var prop = GetStringDataProperty(elemBase, matched);
            if (prop !== undefined) {
                ;;
                replacement = prop;
            } else {
                elemBase = undefined;
            }
        }
        if (!IsObject(elemBase))
            replacement = RegExpGetFunctionalReplacement(result, S, position, replaceValue);
        accumulatedResult += Substring(S, nextSourcePosition,
                                       position - nextSourcePosition) + replacement;
        nextSourcePosition = lastIndex;
        if (matchLength === 0) {
            lastIndex = fullUnicode ? AdvanceStringIndex(S, lastIndex) : lastIndex + 1;
            if (lastIndex > lengthS)
                break;
            lastIndex |= 0;
        }
        if (UnsafeGetStringFromReservedSlot(rx, 1) !== originalSource ||
            UnsafeGetInt32FromReservedSlot(rx, 2) !== originalFlags)
        {
            rx = regexp_construct_raw_flags(originalSource, originalFlags);
        }
    }
    if (nextSourcePosition >= lengthS)
        return accumulatedResult;
    return accumulatedResult + Substring(S, nextSourcePosition, lengthS - nextSourcePosition);
}
function RegExpGlobalReplaceOptSubst(rx, S, lengthS, replaceValue, flags
                   , firstDollarIndex
                  )
{
    var fullUnicode = !!(flags & 0x10);
    var lastIndex = 0;
    rx.lastIndex = 0;
    var accumulatedResult = "";
    var nextSourcePosition = 0;
    while (true) {
        var result = RegExpMatcher(rx, S, lastIndex);
        if (result === null)
            break;
        ;;
        var matched = result[0];
        var matchLength = matched.length | 0;
        var position = result.index | 0;
        lastIndex = position + matchLength;
        var replacement;
        var namedCaptures = result.groups;
        if (namedCaptures !== undefined) {
            namedCaptures = ToObject(namedCaptures);
        }
        replacement = RegExpGetSubstitution(result, S, position, replaceValue,
                                            firstDollarIndex, namedCaptures);
        accumulatedResult += Substring(S, nextSourcePosition,
                                       position - nextSourcePosition) + replacement;
        nextSourcePosition = lastIndex;
        if (matchLength === 0) {
            lastIndex = fullUnicode ? AdvanceStringIndex(S, lastIndex) : lastIndex + 1;
            if (lastIndex > lengthS)
                break;
            lastIndex |= 0;
        }
    }
    if (nextSourcePosition >= lengthS)
        return accumulatedResult;
    return accumulatedResult + Substring(S, nextSourcePosition, lengthS - nextSourcePosition);
}
function RegExpLocalReplaceOpt(rx, S, lengthS, replaceValue
                  )
{
    var lastIndex = ToLength(rx.lastIndex);
    var flags = UnsafeGetInt32FromReservedSlot(rx, 2);
    var globalOrSticky = !!(flags & (0x02 | 0x08));
    if (globalOrSticky) {
        if (lastIndex > lengthS) {
            if (globalOrSticky)
                rx.lastIndex = 0;
            return S;
        }
    } else {
        lastIndex = 0;
    }
    var result = RegExpMatcher(rx, S, lastIndex);
    if (result === null) {
        if (globalOrSticky)
            rx.lastIndex = 0;
        return S;
    }
    ;;
    var matched = result[0];
    var matchLength = matched.length;
    var position = result.index;
    var nextSourcePosition = position + matchLength;
    if (globalOrSticky)
       rx.lastIndex = nextSourcePosition;
    var replacement;
    replacement = replaceValue;
    var accumulatedResult = Substring(S, 0, position) + replacement;
    if (nextSourcePosition >= lengthS)
        return accumulatedResult;
    return accumulatedResult + Substring(S, nextSourcePosition, lengthS - nextSourcePosition);
}
function RegExpLocalReplaceOptShort(rx, S, lengthS, replaceValue
                  )
{
    var lastIndex = ToLength(rx.lastIndex);
    var flags = UnsafeGetInt32FromReservedSlot(rx, 2);
    var globalOrSticky = !!(flags & (0x02 | 0x08));
    if (globalOrSticky) {
        if (lastIndex > lengthS) {
            if (globalOrSticky)
                rx.lastIndex = 0;
            return S;
        }
    } else {
        lastIndex = 0;
    }
    var result = RegExpSearcher(rx, S, lastIndex);
    if (result === -1) {
        if (globalOrSticky)
            rx.lastIndex = 0;
        return S;
    }
    var position = result & 0x7fff;
    var nextSourcePosition = (result >> 15) & 0x7fff;
    if (globalOrSticky)
       rx.lastIndex = nextSourcePosition;
    var replacement;
    replacement = replaceValue;
    var accumulatedResult = Substring(S, 0, position) + replacement;
    if (nextSourcePosition >= lengthS)
        return accumulatedResult;
    return accumulatedResult + Substring(S, nextSourcePosition, lengthS - nextSourcePosition);
}
function RegExpLocalReplaceOptFunc(rx, S, lengthS, replaceValue
                  )
{
    var lastIndex = ToLength(rx.lastIndex);
    var flags = UnsafeGetInt32FromReservedSlot(rx, 2);
    var globalOrSticky = !!(flags & (0x02 | 0x08));
    if (globalOrSticky) {
        if (lastIndex > lengthS) {
            if (globalOrSticky)
                rx.lastIndex = 0;
            return S;
        }
    } else {
        lastIndex = 0;
    }
    var result = RegExpMatcher(rx, S, lastIndex);
    if (result === null) {
        if (globalOrSticky)
            rx.lastIndex = 0;
        return S;
    }
    ;;
    var matched = result[0];
    var matchLength = matched.length;
    var position = result.index;
    var nextSourcePosition = position + matchLength;
    if (globalOrSticky)
       rx.lastIndex = nextSourcePosition;
    var replacement;
    replacement = RegExpGetFunctionalReplacement(result, S, position, replaceValue);
    var accumulatedResult = Substring(S, 0, position) + replacement;
    if (nextSourcePosition >= lengthS)
        return accumulatedResult;
    return accumulatedResult + Substring(S, nextSourcePosition, lengthS - nextSourcePosition);
}
function RegExpLocalReplaceOptSubst(rx, S, lengthS, replaceValue
                   , firstDollarIndex
                  )
{
    var lastIndex = ToLength(rx.lastIndex);
    var flags = UnsafeGetInt32FromReservedSlot(rx, 2);
    var globalOrSticky = !!(flags & (0x02 | 0x08));
    if (globalOrSticky) {
        if (lastIndex > lengthS) {
            if (globalOrSticky)
                rx.lastIndex = 0;
            return S;
        }
    } else {
        lastIndex = 0;
    }
    var result = RegExpMatcher(rx, S, lastIndex);
    if (result === null) {
        if (globalOrSticky)
            rx.lastIndex = 0;
        return S;
    }
    ;;
    var matched = result[0];
    var matchLength = matched.length;
    var position = result.index;
    var nextSourcePosition = position + matchLength;
    if (globalOrSticky)
       rx.lastIndex = nextSourcePosition;
    var replacement;
    var namedCaptures = result.groups;
    if (namedCaptures !== undefined) {
        namedCaptures = ToObject(namedCaptures);
    }
    replacement = RegExpGetSubstitution(result, S, position, replaceValue, firstDollarIndex,
                                        namedCaptures);
    var accumulatedResult = Substring(S, 0, position) + replacement;
    if (nextSourcePosition >= lengthS)
        return accumulatedResult;
    return accumulatedResult + Substring(S, nextSourcePosition, lengthS - nextSourcePosition);
}
function RegExpSearch(string) {
    var rx = this;
    if (!IsObject(rx))
        ThrowTypeError(43, rx === null ? "null" : typeof rx);
    var S = ToString(string);
    var previousLastIndex = rx.lastIndex;
    var lastIndexIsZero = SameValue(previousLastIndex, 0);
    if (!lastIndexIsZero)
        rx.lastIndex = 0;
    if (IsRegExpMethodOptimizable(rx) && S.length < 0x7fff) {
        var result = RegExpSearcher(rx, S, 0);
        if (!lastIndexIsZero) {
            rx.lastIndex = previousLastIndex;
        } else {
            var flags = UnsafeGetInt32FromReservedSlot(rx, 2);
            if (flags & (0x02 | 0x08))
                rx.lastIndex = previousLastIndex;
        }
        if (result === -1)
            return -1;
        return result & 0x7fff;
    }
    return RegExpSearchSlowPath(rx, S, previousLastIndex);
}
function RegExpSearchSlowPath(rx, S, previousLastIndex) {
    var result = RegExpExec(rx, S, false);
    var currentLastIndex = rx.lastIndex;
    if (!SameValue(currentLastIndex, previousLastIndex))
        rx.lastIndex = previousLastIndex;
    if (result === null)
        return -1;
    return result.index;
}
function IsRegExpSplitOptimizable(rx, C) {
    if (!IsRegExpObject(rx))
        return false;
    var RegExpCtor = GetBuiltinConstructor("RegExp");
    if (C !== RegExpCtor)
        return false;
    var RegExpProto = RegExpCtor.prototype;
    return RegExpPrototypeOptimizable(RegExpProto) &&
           RegExpInstanceOptimizable(rx, RegExpProto) &&
           RegExpProto.exec === RegExp_prototype_Exec;
}
function RegExpSplit(string, limit) {
    var rx = this;
    if (!IsObject(rx))
        ThrowTypeError(43, rx === null ? "null" : typeof rx);
    var S = ToString(string);
    var C = SpeciesConstructor(rx, GetBuiltinConstructor("RegExp"));
    var optimizable = IsRegExpSplitOptimizable(rx, C) &&
                      (limit === undefined || typeof limit == "number");
    var flags, unicodeMatching, splitter;
    if (optimizable) {
        flags = UnsafeGetInt32FromReservedSlot(rx, 2);
        unicodeMatching = !!(flags & (0x10));
        if (flags & 0x08) {
            var source = UnsafeGetStringFromReservedSlot(rx, 1);
            splitter = regexp_construct_raw_flags(source, flags & ~0x08);
        } else {
            splitter = rx;
        }
    } else {
        flags = ToString(rx.flags);
        unicodeMatching = callFunction(std_String_includes, flags, "u");
        var newFlags;
        if (callFunction(std_String_includes, flags, "y"))
            newFlags = flags;
        else
            newFlags = flags + "y";
        splitter = new C(rx, newFlags);
    }
    var A = [];
    var lengthA = 0;
    var lim;
    if (limit === undefined)
        lim = 0xffffffff;
    else
        lim = limit >>> 0;
    var p = 0;
    if (lim === 0)
        return A;
    var size = S.length;
    if (size === 0) {
        var z;
        if (optimizable)
            z = RegExpMatcher(splitter, S, 0);
        else
            z = RegExpExec(splitter, S, false);
        if (z !== null)
            return A;
        _DefineDataProperty(A, 0, S);
        return A;
    }
    var q = p;
    while (q < size) {
        var e;
        if (optimizable) {
            z = RegExpMatcher(splitter, S, q);
            if (z === null)
                break;
            q = z.index;
            if (q >= size)
                break;
            e = q + z[0].length;
        } else {
            splitter.lastIndex = q;
            z = RegExpExec(splitter, S, false);
            if (z === null) {
                q = unicodeMatching ? AdvanceStringIndex(S, q) : q + 1;
                continue;
            }
            e = ToLength(splitter.lastIndex);
        }
        if (e === p) {
            q = unicodeMatching ? AdvanceStringIndex(S, q) : q + 1;
            continue;
        }
        _DefineDataProperty(A, lengthA, Substring(S, p, q - p));
        lengthA++;
        if (lengthA === lim)
            return A;
        p = e;
        var numberOfCaptures = std_Math_max(ToLength(z.length) - 1, 0);
        var i = 1;
        while (i <= numberOfCaptures) {
            _DefineDataProperty(A, lengthA, z[i]);
            i++;
            lengthA++;
            if (lengthA === lim)
                return A;
        }
        q = p;
    }
    if (p >= size)
        _DefineDataProperty(A, lengthA, "");
    else
        _DefineDataProperty(A, lengthA, Substring(S, p, size - p));
    return A;
}
function RegExp_prototype_Exec(string) {
    var R = this;
    if (!IsObject(R) || !IsRegExpObject(R))
        return callFunction(CallRegExpMethodIfWrapped, R, string, "RegExp_prototype_Exec");
    var S = ToString(string);
    return RegExpBuiltinExec(R, S, false);
}
function RegExpExec(R, S, forTest) {
    var exec = R.exec;
    if (exec === RegExp_prototype_Exec || !IsCallable(exec)) {
        return RegExpBuiltinExec(R, S, forTest);
    }
    var result = callContentFunction(exec, R, S);
    if (typeof result !== "object")
        ThrowTypeError(467);
    return forTest ? result !== null : result;
}
function RegExpBuiltinExec(R, S, forTest) {
    if (!IsRegExpObject(R))
        return UnwrapAndCallRegExpBuiltinExec(R, S, forTest);
    var lastIndex = ToLength(R.lastIndex);
    var flags = UnsafeGetInt32FromReservedSlot(R, 2);
    var globalOrSticky = !!(flags & (0x02 | 0x08));
    if (!globalOrSticky) {
        lastIndex = 0;
    } else {
        if (lastIndex > S.length) {
            if (globalOrSticky)
                R.lastIndex = 0;
            return forTest ? false : null;
        }
    }
    if (forTest) {
        var endIndex = RegExpTester(R, S, lastIndex);
        if (endIndex == -1) {
            if (globalOrSticky)
                R.lastIndex = 0;
            return false;
        }
        if (globalOrSticky)
            R.lastIndex = endIndex;
        return true;
    }
    var result = RegExpMatcher(R, S, lastIndex);
    if (result === null) {
        if (globalOrSticky)
            R.lastIndex = 0;
    } else {
        if (globalOrSticky)
            R.lastIndex = result.index + result[0].length;
    }
    return result;
}
function UnwrapAndCallRegExpBuiltinExec(R, S, forTest) {
    return callFunction(CallRegExpMethodIfWrapped, R, S, forTest, "CallRegExpBuiltinExec");
}
function CallRegExpBuiltinExec(S, forTest) {
    return RegExpBuiltinExec(this, S, forTest);
}
function RegExpTest(string) {
    var R = this;
    if (!IsObject(R))
        ThrowTypeError(43, R === null ? "null" : typeof R);
    var S = ToString(string);
    return RegExpExec(R, S, true);
}
function $RegExpSpecies() {
    return this;
}
_SetCanonicalName($RegExpSpecies, "get [Symbol.species]");
function IsRegExpMatchAllOptimizable(rx, C) {
    if (!IsRegExpObject(rx))
        return false;
    var RegExpCtor = GetBuiltinConstructor("RegExp");
    if (C !== RegExpCtor)
        return false;
    var RegExpProto = RegExpCtor.prototype;
    return RegExpPrototypeOptimizable(RegExpProto) &&
           RegExpInstanceOptimizable(rx, RegExpProto);
}
function RegExpMatchAll(string) {
    var rx = this;
    if (!IsObject(rx))
        ThrowTypeError(43, rx === null ? "null" : typeof rx);
    var str = ToString(string);
    var C = SpeciesConstructor(rx, GetBuiltinConstructor("RegExp"));
    var source, flags, matcher, lastIndex;
    if (IsRegExpMatchAllOptimizable(rx, C)) {
        source = UnsafeGetStringFromReservedSlot(rx, 1);
        flags = UnsafeGetInt32FromReservedSlot(rx, 2);
        matcher = rx;
        lastIndex = ToLength(rx.lastIndex);
    } else {
        source = "";
        flags = ToString(rx.flags);
        matcher = new C(rx, flags);
        matcher.lastIndex = ToLength(rx.lastIndex);
        flags = (callFunction(std_String_includes, flags, "g") ? 0x02 : 0) |
                (callFunction(std_String_includes, flags, "u") ? 0x10 : 0);
        lastIndex = -2;
    }
    return CreateRegExpStringIterator(matcher, str, source, flags, lastIndex);
}
function CreateRegExpStringIterator(regexp, string, source, flags, lastIndex) {
    ;;
    ;;
    ;;
    ;;
    var iterator = NewRegExpStringIterator();
    UnsafeSetReservedSlot(iterator, 0, regexp);
    UnsafeSetReservedSlot(iterator, 1, string);
    UnsafeSetReservedSlot(iterator, 2, source);
    UnsafeSetReservedSlot(iterator, 3, flags | 0);
    UnsafeSetReservedSlot(iterator, 4, lastIndex);
    return iterator;
}
function IsRegExpStringIteratorNextOptimizable() {
    var RegExpProto = GetBuiltinPrototype("RegExp");
    return RegExpPrototypeOptimizable(RegExpProto) &&
           RegExpProto.exec === RegExp_prototype_Exec;
}
function RegExpStringIteratorNext() {
    var obj;
    if (!IsObject(this) || (obj = GuardToRegExpStringIterator(this)) === null) {
        return callFunction(CallRegExpStringIteratorMethodIfWrapped, this,
                            "RegExpStringIteratorNext");
    }
    var result = { value: undefined, done: false };
    var lastIndex = UnsafeGetReservedSlot(obj, 4);
    if (lastIndex === -1) {
        result.done = true;
        return result;
    }
    var regexp = UnsafeGetObjectFromReservedSlot(obj, 0);
    var string = UnsafeGetStringFromReservedSlot(obj, 1);
    var flags = UnsafeGetInt32FromReservedSlot(obj, 3);
    var global = !!(flags & 0x02);
    var fullUnicode = !!(flags & 0x10);
    if (lastIndex >= 0) {
        ;;
        var source = UnsafeGetStringFromReservedSlot(obj, 2);
        if (IsRegExpStringIteratorNextOptimizable() &&
            UnsafeGetStringFromReservedSlot(regexp, 1) === source &&
            UnsafeGetInt32FromReservedSlot(regexp, 2) === flags)
        {
            var globalOrSticky = !!(flags & (0x02 | 0x08));
            if (!globalOrSticky)
                lastIndex = 0;
            var match = (lastIndex <= string.length)
                        ? RegExpMatcher(regexp, string, lastIndex)
                        : null;
            if (match === null) {
                UnsafeSetReservedSlot(obj, 4,
                                      -1);
                result.done = true;
                return result;
            }
            if (global) {
                var matchLength = match[0].length;
                lastIndex = match.index + matchLength;
                if (matchLength === 0) {
                    lastIndex = fullUnicode ? AdvanceStringIndex(string, lastIndex) : lastIndex + 1;
                }
                UnsafeSetReservedSlot(obj, 4, lastIndex);
            } else {
                UnsafeSetReservedSlot(obj, 4,
                                      -1);
            }
            result.value = match;
            return result;
        }
        regexp = regexp_construct_raw_flags(source, flags);
        regexp.lastIndex = lastIndex;
        UnsafeSetReservedSlot(obj, 0, regexp);
        UnsafeSetReservedSlot(obj, 4,
                              -2);
    }
    var match = RegExpExec(regexp, string, false);
    if (match === null) {
        UnsafeSetReservedSlot(obj, 4,
                              -1);
        result.done = true;
        return result;
    }
    if (global) {
        var matchStr = ToString(match[0]);
        if (matchStr.length === 0) {
            var thisIndex = ToLength(regexp.lastIndex);
            var nextIndex = fullUnicode ? AdvanceStringIndex(string, thisIndex) : thisIndex + 1;
            regexp.lastIndex = nextIndex;
        }
    } else {
        UnsafeSetReservedSlot(obj, 4,
                              -1);
    }
    result.value = match;
    return result;
}
function IsRegExp(argument) {
    if (!IsObject(argument)) {
        return false;
    }
    var matcher = argument[std_match];
    if (matcher !== undefined) {
        return !!matcher;
    }
    return IsPossiblyWrappedRegExpObject(argument);
}
function FUNC_NAME(rx, S, lengthS, replaceValue, flags
                  )
{
    var fullUnicode = !!(flags & 0x10);
    var lastIndex = 0;
    rx.lastIndex = 0;
    var accumulatedResult = "";
    var nextSourcePosition = 0;
    while (true) {
        var result = RegExpMatcher(rx, S, lastIndex);
        if (result === null)
            break;
        ;;
        var matched = result[0];
        var matchLength = matched.length | 0;
        var position = result.index | 0;
        lastIndex = position + matchLength;
        var replacement;
        replacement = replaceValue;
        accumulatedResult += Substring(S, nextSourcePosition,
                                       position - nextSourcePosition) + replacement;
        nextSourcePosition = lastIndex;
        if (matchLength === 0) {
            lastIndex = fullUnicode ? AdvanceStringIndex(S, lastIndex) : lastIndex + 1;
            if (lastIndex > lengthS)
                break;
            lastIndex |= 0;
        }
    }
    if (nextSourcePosition >= lengthS)
        return accumulatedResult;
    return accumulatedResult + Substring(S, nextSourcePosition, lengthS - nextSourcePosition);
}
function FUNC_NAME(rx, S, lengthS, replaceValue
                  )
{
    var lastIndex = ToLength(rx.lastIndex);
    var flags = UnsafeGetInt32FromReservedSlot(rx, 2);
    var globalOrSticky = !!(flags & (0x02 | 0x08));
    if (globalOrSticky) {
        if (lastIndex > lengthS) {
            if (globalOrSticky)
                rx.lastIndex = 0;
            return S;
        }
    } else {
        lastIndex = 0;
    }
    var result = RegExpMatcher(rx, S, lastIndex);
    if (result === null) {
        if (globalOrSticky)
            rx.lastIndex = 0;
        return S;
    }
    ;;
    var matched = result[0];
    var matchLength = matched.length;
    var position = result.index;
    var nextSourcePosition = position + matchLength;
    if (globalOrSticky)
       rx.lastIndex = nextSourcePosition;
    var replacement;
    replacement = replaceValue;
    var accumulatedResult = Substring(S, 0, position) + replacement;
    if (nextSourcePosition >= lengthS)
        return accumulatedResult;
    return accumulatedResult + Substring(S, nextSourcePosition, lengthS - nextSourcePosition);
}
function StringProtoHasNoMatch() {
    var ObjectProto = GetBuiltinPrototype("Object");
    var StringProto = GetBuiltinPrototype("String");
    if (!ObjectHasPrototype(StringProto, ObjectProto))
        return false;
    return !(std_match in StringProto);
}
function IsStringMatchOptimizable() {
    var RegExpProto = GetBuiltinPrototype("RegExp");
    return RegExpPrototypeOptimizable(RegExpProto) &&
           RegExpProto.exec === RegExp_prototype_Exec &&
           RegExpProto[std_match] === RegExpMatch;
}
function String_match(regexp) {
    RequireObjectCoercible(this);
    var isPatternString = (typeof regexp === "string");
    if (!(isPatternString && StringProtoHasNoMatch()) && regexp !== undefined && regexp !== null) {
        var matcher = GetMethod(regexp, std_match);
        if (matcher !== undefined)
            return callContentFunction(matcher, regexp, this);
    }
    var S = ToString(this);
    if (isPatternString && IsStringMatchOptimizable()) {
        var flatResult = FlatStringMatch(S, regexp);
        if (flatResult !== undefined)
            return flatResult;
    }
    var rx = RegExpCreate(regexp);
    if (IsStringMatchOptimizable())
        return RegExpMatcher(rx, S, 0);
    return callContentFunction(GetMethod(rx, std_match), rx, S);
}
function String_matchAll(regexp) {
    RequireObjectCoercible(this);
    if (regexp !== undefined && regexp !== null) {
        if (IsRegExp(regexp)) {
            var flags = regexp.flags;
            if (flags === undefined || flags === null) {
                ThrowTypeError(94);
            }
            if (!callFunction(std_String_includes, ToString(flags), "g")) {
                ThrowTypeError(95, "matchAll");
            }
        }
        var matcher = GetMethod(regexp, std_matchAll);
        if (matcher !== undefined)
            return callContentFunction(matcher, regexp, this);
    }
    var string = ToString(this);
    var rx = RegExpCreate(regexp, "g");
    return callContentFunction(GetMethod(rx, std_matchAll), rx, string);
}
function String_pad(maxLength, fillString, padEnd) {
    RequireObjectCoercible(this);
    let str = ToString(this);
    let intMaxLength = ToLength(maxLength);
    let strLen = str.length;
    if (intMaxLength <= strLen)
        return str;
    ;;
    let filler = ToString(fillString);
    if (filler === "")
        return str;
    if (intMaxLength > ((1 << 30) - 2))
        ThrowRangeError(93);
    let fillLen = intMaxLength - strLen;
    let truncatedStringFiller = callFunction(String_repeat, filler,
                                             (fillLen / filler.length) | 0);
    truncatedStringFiller += Substring(filler, 0, fillLen % filler.length);
    if (padEnd === true)
        return str + truncatedStringFiller;
    return truncatedStringFiller + str;
}
function String_pad_start(maxLength, fillString = " ") {
    return callFunction(String_pad, this, maxLength, fillString, false);
}
function String_pad_end(maxLength, fillString = " ") {
    return callFunction(String_pad, this, maxLength, fillString, true);
}
function StringProtoHasNoReplace() {
    var ObjectProto = GetBuiltinPrototype("Object");
    var StringProto = GetBuiltinPrototype("String");
    if (!ObjectHasPrototype(StringProto, ObjectProto))
        return false;
    return !(std_replace in StringProto);
}
function Substring(str, from, length) {
    ;;
    ;;
    ;;
    return SubstringKernel(str, from | 0, length | 0);
}
function String_replace(searchValue, replaceValue) {
    RequireObjectCoercible(this);
    if (!(typeof searchValue === "string" && StringProtoHasNoReplace()) &&
        searchValue !== undefined && searchValue !== null)
    {
        var replacer = GetMethod(searchValue, std_replace);
        if (replacer !== undefined)
            return callContentFunction(replacer, searchValue, this, replaceValue);
    }
    var string = ToString(this);
    var searchString = ToString(searchValue);
    if (typeof replaceValue === "string") {
        return StringReplaceString(string, searchString, replaceValue);
    }
    if (!IsCallable(replaceValue)) {
        return StringReplaceString(string, searchString, ToString(replaceValue));
    }
    var pos = callFunction(std_String_indexOf, string, searchString);
    if (pos === -1)
        return string;
    var replStr = ToString(callContentFunction(replaceValue, undefined, searchString, pos, string));
    var tailPos = pos + searchString.length;
    var newString;
    if (pos === 0)
        newString = "";
    else
        newString = Substring(string, 0, pos);
    newString += replStr;
    var stringLength = string.length;
    if (tailPos < stringLength)
        newString += Substring(string, tailPos, stringLength - tailPos);
    return newString;
}
function String_replaceAll(searchValue, replaceValue) {
    RequireObjectCoercible(this);
    if (searchValue !== undefined && searchValue !== null) {
        if (IsRegExp(searchValue)) {
            var flags = searchValue.flags;
            if (flags === undefined || flags === null) {
                ThrowTypeError(94);
            }
            if (!callFunction(std_String_includes, ToString(flags), "g")) {
                ThrowTypeError(95, "replaceAll");
            }
        }
        var replacer = GetMethod(searchValue, std_replace);
        if (replacer !== undefined) {
            return callContentFunction(replacer, searchValue, this, replaceValue);
        }
    }
    var string = ToString(this);
    var searchString = ToString(searchValue);
    if (!IsCallable(replaceValue)) {
        return StringReplaceAllString(string, searchString, ToString(replaceValue));
    }
    var searchLength = searchString.length;
    var advanceBy = std_Math_max(1, searchLength);
    var endOfLastMatch = 0;
    var result = "";
    var position = 0;
    while (true) {
        var nextPosition = callFunction(std_String_indexOf, string, searchString, position);
        if (nextPosition < position) {
            break;
        }
        position = nextPosition;
        var replacement = ToString(callContentFunction(replaceValue, undefined, searchString,
                                                       position, string));
        var stringSlice = Substring(string, endOfLastMatch, position - endOfLastMatch);
        result += stringSlice + replacement;
        endOfLastMatch = position + searchLength;
        position += advanceBy;
    }
    if (endOfLastMatch < string.length) {
        result += Substring(string, endOfLastMatch, string.length - endOfLastMatch);
    }
    return result;
}
function StringProtoHasNoSearch() {
    var ObjectProto = GetBuiltinPrototype("Object");
    var StringProto = GetBuiltinPrototype("String");
    if (!ObjectHasPrototype(StringProto, ObjectProto))
        return false;
    return !(std_search in StringProto);
}
function IsStringSearchOptimizable() {
    var RegExpProto = GetBuiltinPrototype("RegExp");
    return RegExpPrototypeOptimizable(RegExpProto) &&
           RegExpProto.exec === RegExp_prototype_Exec &&
           RegExpProto[std_search] === RegExpSearch;
}
function String_search(regexp) {
    RequireObjectCoercible(this);
    var isPatternString = (typeof regexp === "string");
    if (!(isPatternString && StringProtoHasNoSearch()) && regexp !== undefined && regexp !== null) {
        var searcher = GetMethod(regexp, std_search);
        if (searcher !== undefined)
            return callContentFunction(searcher, regexp, this);
    }
    var string = ToString(this);
    if (isPatternString && IsStringSearchOptimizable()) {
        var flatResult = FlatStringSearch(string, regexp);
        if (flatResult !== -2)
            return flatResult;
    }
    var rx = RegExpCreate(regexp);
    return callContentFunction(GetMethod(rx, std_search), rx, string);
}
function StringProtoHasNoSplit() {
    var ObjectProto = GetBuiltinPrototype("Object");
    var StringProto = GetBuiltinPrototype("String");
    if (!ObjectHasPrototype(StringProto, ObjectProto))
        return false;
    return !(std_split in StringProto);
}
function String_split(separator, limit) {
    RequireObjectCoercible(this);
    if (typeof this === "string") {
        if (StringProtoHasNoSplit()) {
            if (typeof separator === "string") {
                if (limit === undefined) {
                    return StringSplitString(this, separator);
                }
            }
        }
    }
    if (!(typeof separator == "string" && StringProtoHasNoSplit()) &&
        separator !== undefined && separator !== null)
    {
        var splitter = GetMethod(separator, std_split);
        if (splitter !== undefined)
            return callContentFunction(splitter, separator, this, limit);
    }
    var S = ToString(this);
    var R;
    if (limit !== undefined) {
        var lim = limit >>> 0;
        R = ToString(separator);
        if (lim === 0)
            return [];
        if (separator === undefined)
            return [S];
        return StringSplitStringLimit(S, R, lim);
    }
    R = ToString(separator);
    if (separator === undefined)
        return [S];
    return StringSplitString(S, R);
}
function String_substring(start, end) {
    RequireObjectCoercible(this);
    var str = ToString(this);
    var len = str.length;
    var intStart = ToInteger(start);
    var intEnd = (end === undefined) ? len : ToInteger(end);
    var finalStart = std_Math_min(std_Math_max(intStart, 0), len);
    var finalEnd = std_Math_min(std_Math_max(intEnd, 0), len);
    var from, to;
    if (finalStart < finalEnd) {
        from = finalStart;
        to = finalEnd;
    } else {
        from = finalEnd;
        to = finalStart;
    }
    return SubstringKernel(str, from | 0, (to - from) | 0);
}
function String_substr(start, length) {
    RequireObjectCoercible(this);
    var str = ToString(this);
    var intStart = ToInteger(start);
    var size = str.length;
    var end = (length === undefined) ? size : ToInteger(length);
    if (intStart < 0)
        intStart = std_Math_max(intStart + size, 0);
    var resultLength = std_Math_min(std_Math_max(end, 0), size - intStart);
    if (resultLength <= 0)
        return "";
    return SubstringKernel(str, intStart | 0, resultLength | 0);
}
function String_slice(start, end) {
    RequireObjectCoercible(this);
    var str = ToString(this);
    var len = str.length;
    var intStart = ToInteger(start);
    var intEnd = (end === undefined) ? len : ToInteger(end);
    var from = (intStart < 0) ? std_Math_max(len + intStart, 0) : std_Math_min(intStart, len);
    var to = (intEnd < 0) ? std_Math_max(len + intEnd, 0) : std_Math_min(intEnd, len);
    var span = std_Math_max(to - from, 0);
    return SubstringKernel(str, from | 0, span | 0);
}
function String_codePointAt(pos) {
    RequireObjectCoercible(this);
    var S = ToString(this);
    var position = ToInteger(pos);
    var size = S.length;
    if (position < 0 || position >= size)
        return undefined;
    var first = callFunction(std_String_charCodeAt, S, position);
    if (first < 0xD800 || first > 0xDBFF || position + 1 === size)
        return first;
    var second = callFunction(std_String_charCodeAt, S, position + 1);
    if (second < 0xDC00 || second > 0xDFFF)
        return first;
    return (first - 0xD800) * 0x400 + (second - 0xDC00) + 0x10000;
}
function String_repeat(count) {
    RequireObjectCoercible(this);
    var S = ToString(this);
    var n = ToInteger(count);
    if (n < 0)
        ThrowRangeError(91);
    if (!(n * S.length <= ((1 << 30) - 2)))
        ThrowRangeError(93);
    ;;
    ;;
    n = n & (((1 << 30) - 2) + 1);
    var T = "";
    for (;;) {
        if (n & 1)
            T += S;
        n >>= 1;
        if (n)
            S += S;
        else
            break;
    }
    return T;
}
function String_iterator() {
    RequireObjectCoercible(this);
    var S = ToString(this);
    var iterator = NewStringIterator();
    UnsafeSetReservedSlot(iterator, 0, S);
    UnsafeSetReservedSlot(iterator, 1, 0);
    return iterator;
}
function StringIteratorNext() {
    var obj;
    if (!IsObject(this) || (obj = GuardToStringIterator(this)) === null) {
        return callFunction(CallStringIteratorMethodIfWrapped, this,
                            "StringIteratorNext");
    }
    var S = UnsafeGetStringFromReservedSlot(obj, 0);
    var index = UnsafeGetInt32FromReservedSlot(obj, 1);
    var size = S.length;
    var result = { value: undefined, done: false };
    if (index >= size) {
        result.done = true;
        return result;
    }
    var charCount = 1;
    var first = callFunction(std_String_charCodeAt, S, index);
    if (first >= 0xD800 && first <= 0xDBFF && index + 1 < size) {
        var second = callFunction(std_String_charCodeAt, S, index + 1);
        if (second >= 0xDC00 && second <= 0xDFFF) {
            first = (first - 0xD800) * 0x400 + (second - 0xDC00) + 0x10000;
            charCount = 2;
        }
    }
    UnsafeSetReservedSlot(obj, 1, index + charCount);
    result.value = callFunction(std_String_fromCodePoint, null, first & 0x1fffff);
    return result;
}
function String_static_raw(callSite) {
    var cooked = ToObject(callSite);
    var raw = ToObject(cooked.raw);
    var literalSegments = ToLength(raw.length);
    if (literalSegments === 0)
        return "";
    if (literalSegments === 1)
        return ToString(raw[0]);
    var resultString = ToString(raw[0]);
    for (var nextIndex = 1; nextIndex < literalSegments; nextIndex++) {
        if (nextIndex < arguments.length)
            resultString += ToString(arguments[nextIndex]);
        resultString += ToString(raw[nextIndex]);
    }
    return resultString;
}
function String_big() {
    RequireObjectCoercible(this);
    return "<big>" + ToString(this) + "</big>";
}
function String_blink() {
    RequireObjectCoercible(this);
    return "<blink>" + ToString(this) + "</blink>";
}
function String_bold() {
    RequireObjectCoercible(this);
    return "<b>" + ToString(this) + "</b>";
}
function String_fixed() {
    RequireObjectCoercible(this);
    return "<tt>" + ToString(this) + "</tt>";
}
function String_italics() {
    RequireObjectCoercible(this);
    return "<i>" + ToString(this) + "</i>";
}
function String_small() {
    RequireObjectCoercible(this);
    return "<small>" + ToString(this) + "</small>";
}
function String_strike() {
    RequireObjectCoercible(this);
    return "<strike>" + ToString(this) + "</strike>";
}
function String_sub() {
    RequireObjectCoercible(this);
    return "<sub>" + ToString(this) + "</sub>";
}
function String_sup() {
    RequireObjectCoercible(this);
    return "<sup>" + ToString(this) + "</sup>";
}
function EscapeAttributeValue(v) {
    var inputStr = ToString(v);
    return StringReplaceAllString(inputStr, '"', "&quot;");
}
function String_anchor(name) {
    RequireObjectCoercible(this);
    var S = ToString(this);
    return '<a name="' + EscapeAttributeValue(name) + '">' + S + "</a>";
}
function String_fontcolor(color) {
    RequireObjectCoercible(this);
    var S = ToString(this);
    return '<font color="' + EscapeAttributeValue(color) + '">' + S + "</font>";
}
function String_fontsize(size) {
    RequireObjectCoercible(this);
    var S = ToString(this);
    return '<font size="' + EscapeAttributeValue(size) + '">' + S + "</font>";
}
function String_link(url) {
    RequireObjectCoercible(this);
    var S = ToString(this);
    return '<a href="' + EscapeAttributeValue(url) + '">' + S + "</a>";
}
function SetConstructorInit(iterable) {
    var set = this;
    var adder = set.add;
    if (!IsCallable(adder))
        ThrowTypeError(9, typeof adder);
    for (var nextValue of allowContentIter(iterable))
        callContentFunction(adder, set, nextValue);
}
function SetForEach(callbackfn, thisArg = undefined) {
    var S = this;
    if (!IsObject(S) || (S = GuardToSetObject(S)) === null)
        return callFunction(CallSetMethodIfWrapped, this, callbackfn, thisArg, "SetForEach");
    if (!IsCallable(callbackfn))
        ThrowTypeError(9, DecompileArg(0, callbackfn));
    var values = callFunction(std_Set_iterator, S);
    var setIterationResult = setIteratorTemp.setIterationResult;
    if (!setIterationResult)
        setIterationResult = setIteratorTemp.setIterationResult = _CreateSetIterationResult();
    while (true) {
        var done = _GetNextSetEntryForIterator(values, setIterationResult);
        if (done)
            break;
        var value = setIterationResult[0];
        setIterationResult[0] = null;
        callContentFunction(callbackfn, thisArg, value, value, S);
    }
}
function $SetValues() {
    return callFunction(std_Set_iterator, this);
}
_SetCanonicalName($SetValues, "values");
function $SetSpecies() {
    return this;
}
_SetCanonicalName($SetSpecies, "get [Symbol.species]");
var setIteratorTemp = { setIterationResult: null };
function SetIteratorNext() {
    var O = this;
    if (!IsObject(O) || (O = GuardToSetIterator(O)) === null)
        return callFunction(CallSetIteratorMethodIfWrapped, this, "SetIteratorNext");
    var setIterationResult = setIteratorTemp.setIterationResult;
    if (!setIterationResult)
        setIterationResult = setIteratorTemp.setIterationResult = _CreateSetIterationResult();
    var retVal = {value: undefined, done: true};
    var done = _GetNextSetEntryForIterator(O, setIterationResult);
    if (!done) {
        var itemKind = UnsafeGetInt32FromReservedSlot(O, 2);
        var result;
        if (itemKind === 1) {
            result = setIterationResult[0];
        } else {
            ;;
            result = [setIterationResult[0], setIterationResult[0]];
        }
        setIterationResult[0] = null;
        retVal.value = result;
        retVal.done = false;
    }
    return retVal;
}
function CountingSort(array, len, signed, comparefn) {
    ;;
    if (len < 128) {
        QuickSort(array, len, comparefn);
        return array;
    }
    var min = 0;
    if (signed) {
        min = -128;
    }
    var buffer = [
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    ];
    for (var i = 0; i < len; i++) {
        var val = array[i];
        buffer[val - min]++;
    }
    var val = -1;
    for (var i = 0; i < len;) {
        var j;
        do {
            j = buffer[++val];
        } while (j === 0);
        for (; j > 0; j--)
            array[i++] = val + min;
    }
    return array;
}
function ByteAtCol(x, pos) {
    return (x >> (pos * 8)) & 0xFF;
}
function SortByColumn(array, len, aux, col, counts) {
    const R = 256;
    ;;
    for (let r = 0; r < R + 1; r++) {
        counts[r] = 0;
    }
    for (let i = 0; i < len; i++) {
        let val = array[i];
        let b = ByteAtCol(val, col);
        counts[b + 1]++;
    }
    for (let r = 0; r < R; r++) {
        counts[r + 1] += counts[r];
    }
    for (let i = 0; i < len; i++) {
        let val = array[i];
        let b = ByteAtCol(val, col);
        aux[counts[b]++] = val;
    }
    for (let i = 0; i < len; i++) {
        array[i] = aux[i];
    }
}
function RadixSort(array, len, buffer, nbytes, signed, floating, comparefn) {
    ;;
    if (len < 512) {
        QuickSort(array, len, comparefn);
        return array;
    }
    let aux = [];
    for (let i = 0; i < len; i++)
        _DefineDataProperty(aux, i, 0);
    let view = array;
    let signMask = 1 << nbytes * 8 - 1;
    if (floating) {
        if (buffer === null) {
            buffer = callFunction(std_TypedArray_buffer, array);
            ;;
        }
        let offset = IsTypedArray(array)
                     ? TypedArrayByteOffset(array)
                     : callFunction(CallTypedArrayMethodIfWrapped, array,
                                    "TypedArrayByteOffsetMethod");
        view = new Int32Array(buffer, offset, len);
        for (let i = 0; i < len; i++) {
            if (view[i] & signMask) {
                if ((view[i] & 0x7F800000) !== 0x7F800000 || (view[i] & 0x007FFFFF) === 0) {
                    view[i] ^= 0xFFFFFFFF;
                }
            } else {
                view[i] ^= signMask;
            }
        }
    } else if (signed) {
        for (let i = 0; i < len; i++) {
            view[i] ^= signMask;
        }
    }
    let counts = [
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,
    ];
    for (let col = 0; col < nbytes; col++) {
        SortByColumn(view, len, aux, col, counts);
    }
    if (floating) {
        for (let i = 0; i < len; i++) {
            if (view[i] & signMask) {
                view[i] ^= signMask;
            } else {
                view[i] ^= 0xFFFFFFFF;
            }
        }
    } else if (signed) {
        for (let i = 0; i < len; i++) {
            view[i] ^= signMask;
        }
    }
    return array;
}
function InsertionSort(array, from, to, comparefn) {
    let item, swap, i, j;
    for (i = from + 1; i <= to; i++) {
        item = array[i];
        for (j = i - 1; j >= from; j--) {
            swap = array[j];
            if (comparefn(swap, item) <= 0)
                break;
            array[j + 1] = swap;
        }
        array[j + 1] = item;
    }
}
function SwapArrayElements(array, i, j) {
    var swap = array[i];
    array[i] = array[j];
    array[j] = swap;
}
function Merge(list, out, start, mid, end, comparefn) {
    if (mid >= end || comparefn(list[mid], list[mid + 1]) <= 0) {
        for (var i = start; i <= end; i++) {
            _DefineDataProperty(out, i, list[i]);
        }
        return;
    }
    var i = start;
    var j = mid + 1;
    var k = start;
    while (i <= mid && j <= end) {
        var lvalue = list[i];
        var rvalue = list[j];
        if (comparefn(lvalue, rvalue) <= 0) {
            _DefineDataProperty(out, k++, lvalue);
            i++;
        } else {
            _DefineDataProperty(out, k++, rvalue);
            j++;
        }
    }
    while (i <= mid) {
        _DefineDataProperty(out, k++, list[i++]);
    }
    while (j <= end) {
        _DefineDataProperty(out, k++, list[j++]);
    }
}
function MoveHoles(sparse, sparseLen, dense, denseLen) {
    for (var i = 0; i < denseLen; i++)
        sparse[i] = dense[i];
    for (var j = denseLen; j < sparseLen; j++)
        delete sparse[j];
}
function MergeSort(array, len, comparefn) {
    var denseList = [];
    var denseLen = 0;
    for (var i = 0; i < len; i++) {
        if (i in array)
            _DefineDataProperty(denseList, denseLen++, array[i]);
    }
    if (denseLen < 1)
        return array;
    if (denseLen < 24) {
        InsertionSort(denseList, 0, denseLen - 1, comparefn);
        MoveHoles(array, len, denseList, denseLen);
        return array;
    }
    var lBuffer = denseList;
    var rBuffer = [];
    var windowSize = 4;
    for (var start = 0; start < denseLen - 1; start += windowSize) {
        var end = std_Math_min(start + windowSize - 1, denseLen - 1);
        InsertionSort(lBuffer, start, end, comparefn);
    }
    for (; windowSize < denseLen; windowSize = 2 * windowSize) {
        for (var start = 0; start < denseLen; start += 2 * windowSize) {
            var mid = start + windowSize - 1;
            var end = std_Math_min(start + 2 * windowSize - 1, denseLen - 1);
            Merge(lBuffer, rBuffer, start, mid, end, comparefn);
        }
        var swap = lBuffer;
        lBuffer = rBuffer;
        rBuffer = swap;
    }
    MoveHoles(array, len, lBuffer, denseLen);
    return array;
}
function MergeTypedArray(list, out, start, mid, end, comparefn) {
    if (mid >= end || comparefn(list[mid], list[mid + 1]) <= 0) {
        for (var i = start; i <= end; i++) {
            out[i] = list[i];
        }
        return;
    }
    var i = start;
    var j = mid + 1;
    var k = start;
    while (i <= mid && j <= end) {
        var lvalue = list[i];
        var rvalue = list[j];
        if (comparefn(lvalue, rvalue) <= 0) {
            out[k++] = lvalue;
            i++;
        } else {
            out[k++] = rvalue;
            j++;
        }
    }
    while (i <= mid) {
        out[k++] = list[i++];
    }
    while (j <= end) {
        out[k++] = list[j++];
    }
}
function MergeSortTypedArray(array, len, comparefn) {
    ;;
    if (len < 8) {
        InsertionSort(array, 0, len - 1, comparefn);
        return array;
    }
    var C = _ConstructorForTypedArray(array);
    var lBuffer = array;
    var rBuffer = new C(len);
    var windowSize = 4;
    for (var start = 0; start < len - 1; start += windowSize) {
        var end = std_Math_min(start + windowSize - 1, len - 1);
        InsertionSort(lBuffer, start, end, comparefn);
    }
    for (; windowSize < len; windowSize = 2 * windowSize) {
        for (var start = 0; start < len; start += 2 * windowSize) {
            var mid = start + windowSize - 1;
            var end = std_Math_min(start + 2 * windowSize - 1, len - 1);
            MergeTypedArray(lBuffer, rBuffer, start, mid, end, comparefn);
        }
        var swap = lBuffer;
        lBuffer = rBuffer;
        rBuffer = swap;
    }
    if (lBuffer !== array) {
        for (var i = 0; i < len; i++) {
            array[i] = lBuffer[i];
        }
    }
    return array;
}
function Partition(array, from, to, comparefn) {
    ;;
    var medianIndex = from + ((to - from) >> 1);
    var i = from + 1;
    var j = to;
    SwapArrayElements(array, medianIndex, i);
    if (comparefn(array[from], array[to]) > 0)
        SwapArrayElements(array, from, to);
    if (comparefn(array[i], array[to]) > 0)
        SwapArrayElements(array, i, to);
    if (comparefn(array[from], array[i]) > 0)
        SwapArrayElements(array, from, i);
    var pivotIndex = i;
    for (;;) {
        do i++; while (comparefn(array[i], array[pivotIndex]) < 0);
        do j--; while (comparefn(array[j], array[pivotIndex]) > 0);
        if (i > j)
            break;
        SwapArrayElements(array, i, j);
    }
    SwapArrayElements(array, pivotIndex, j);
    return j;
}
function QuickSort(array, len, comparefn) {
    ;;
    var stack = new List();
    var top = 0;
    var start = 0;
    var end   = len - 1;
    var pivotIndex, leftLen, rightLen;
    for (;;) {
        if (end - start <= 23) {
            InsertionSort(array, start, end, comparefn);
            if (top < 1)
                break;
            end   = stack[--top];
            start = stack[--top];
        } else {
            pivotIndex = Partition(array, start, end, comparefn);
            leftLen = (pivotIndex - 1) - start;
            rightLen = end - (pivotIndex + 1);
            if (rightLen > leftLen) {
                stack[top++] = start;
                stack[top++] = pivotIndex - 1;
                start = pivotIndex + 1;
            } else {
                stack[top++] = pivotIndex + 1;
                stack[top++] = end;
                end = pivotIndex - 1;
            }
        }
    }
    return array;
}
function ViewedArrayBufferIfReified(tarray) {
    ;;
    var buf = UnsafeGetReservedSlot(tarray, 0);
    ;;
    return buf;
}
function IsDetachedBuffer(buffer) {
    if (buffer === null)
        return false;
    ;;
    if ((buffer = GuardToArrayBuffer(buffer)) === null)
        return false;
    var flags = UnsafeGetInt32FromReservedSlot(buffer, 3);
    return (flags & 0x8) !== 0;
}
function TypedArrayLengthMethod() {
    return TypedArrayLength(this);
}
function TypedArrayByteOffsetMethod() {
    return TypedArrayByteOffset(this);
}
function GetAttachedArrayBuffer(tarray) {
    var buffer = ViewedArrayBufferIfReified(tarray);
    if (IsDetachedBuffer(buffer))
        ThrowTypeError(509);
    return buffer;
}
function GetAttachedArrayBufferMethod() {
    return GetAttachedArrayBuffer(this);
}
function IsTypedArrayEnsuringArrayBuffer(arg) {
    if (IsObject(arg) && IsTypedArray(arg)) {
        GetAttachedArrayBuffer(arg);
        return true;
    }
    callFunction(CallTypedArrayMethodIfWrapped, arg, "GetAttachedArrayBufferMethod");
    return false;
}
function TypedArraySpeciesConstructor(obj) {
    ;;
    var ctor = obj.constructor;
    if (ctor === undefined)
        return _ConstructorForTypedArray(obj);
    if (!IsObject(ctor))
        ThrowTypeError(43, "object's 'constructor' property");
    var s = ctor[std_species];
    if (s === undefined || s === null)
        return _ConstructorForTypedArray(obj);
    if (IsConstructor(s))
        return s;
    ThrowTypeError(10, "@@species property of object's constructor");
}
function ValidateTypedArray(obj) {
    if (IsObject(obj)) {
        if (IsTypedArray(obj)) {
            GetAttachedArrayBuffer(obj);
            return true;
        }
        if (IsPossiblyWrappedTypedArray(obj)) {
            if (PossiblyWrappedTypedArrayHasDetachedBuffer(obj))
                ThrowTypeError(509);
            return false;
        }
    }
    ThrowTypeError(512);
}
function TypedArrayCreateWithLength(constructor, length) {
    var newTypedArray = new constructor(length);
    var isTypedArray = ValidateTypedArray(newTypedArray);
    var len;
    if (isTypedArray) {
        len = TypedArrayLength(newTypedArray);
    } else {
        len = callFunction(CallTypedArrayMethodIfWrapped, newTypedArray,
                           "TypedArrayLengthMethod");
    }
    if (len < length)
        ThrowTypeError(513, length, len);
    return newTypedArray;
}
function TypedArrayCreateWithBuffer(constructor, buffer, byteOffset, length) {
    var newTypedArray = new constructor(buffer, byteOffset, length);
    ValidateTypedArray(newTypedArray);
    return newTypedArray;
}
function TypedArraySpeciesCreateWithLength(exemplar, length) {
    var C = TypedArraySpeciesConstructor(exemplar);
    return TypedArrayCreateWithLength(C, length);
}
function TypedArraySpeciesCreateWithBuffer(exemplar, buffer, byteOffset, length) {
    var C = TypedArraySpeciesConstructor(exemplar);
    return TypedArrayCreateWithBuffer(C, buffer, byteOffset, length);
}
function TypedArrayCopyWithin(target, start, end = undefined) {
    if (!IsObject(this) || !IsTypedArray(this)) {
        return callFunction(CallTypedArrayMethodIfWrapped, this, target, start, end,
                            "TypedArrayCopyWithin");
    }
    GetAttachedArrayBuffer(this);
    var obj = this;
    var len = TypedArrayLength(obj);
    ;;
    var relativeTarget = ToInteger(target);
    var to = relativeTarget < 0 ? std_Math_max(len + relativeTarget, 0)
                                : std_Math_min(relativeTarget, len);
    var relativeStart = ToInteger(start);
    var from = relativeStart < 0 ? std_Math_max(len + relativeStart, 0)
                                 : std_Math_min(relativeStart, len);
    var relativeEnd = end === undefined ? len : ToInteger(end);
    var final = relativeEnd < 0 ? std_Math_max(len + relativeEnd, 0)
                                : std_Math_min(relativeEnd, len);
    var count = std_Math_min(final - from, len - to);
    ;;
    ;;
    ;;
    if (count > 0)
        MoveTypedArrayElements(obj, to | 0, from | 0, count | 0);
    return obj;
}
function TypedArrayEntries() {
    var O = this;
    IsTypedArrayEnsuringArrayBuffer(O);
    return CreateArrayIterator(O, 2);
}
function TypedArrayEvery(callbackfn) {
    var O = this;
    var isTypedArray = IsTypedArrayEnsuringArrayBuffer(O);
    var len;
    if (isTypedArray)
        len = TypedArrayLength(O);
    else
        len = callFunction(CallTypedArrayMethodIfWrapped, O, "TypedArrayLengthMethod");
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "%TypedArray%.prototype.every");
    if (!IsCallable(callbackfn))
        ThrowTypeError(9, DecompileArg(0, callbackfn));
    var T = arguments.length > 1 ? arguments[1] : void 0;
    for (var k = 0; k < len; k++) {
        var kValue = O[k];
        var testResult = callContentFunction(callbackfn, T, kValue, k, O);
        if (!testResult)
            return false;
    }
    return true;
}
function TypedArrayFill(value, start = 0, end = undefined) {
    if (!IsObject(this) || !IsTypedArray(this)) {
        return callFunction(CallTypedArrayMethodIfWrapped, this, value, start, end,
                            "TypedArrayFill");
    }
    var O = this;
    var buffer = GetAttachedArrayBuffer(this);
    var len = TypedArrayLength(O);
    var kind = GetTypedArrayKind(O);
    if (kind === 9 || kind === 10) {
        value = ToBigInt(value);
    } else {
        value = ToNumber(value);
    }
    var relativeStart = ToInteger(start);
    var k = relativeStart < 0
            ? std_Math_max(len + relativeStart, 0)
            : std_Math_min(relativeStart, len);
    var relativeEnd = end === undefined ? len : ToInteger(end);
    var final = relativeEnd < 0
                ? std_Math_max(len + relativeEnd, 0)
                : std_Math_min(relativeEnd, len);
    if (buffer === null) {
        buffer = ViewedArrayBufferIfReified(O);
    }
    if (IsDetachedBuffer(buffer))
        ThrowTypeError(509);
    for (; k < final; k++) {
        O[k] = value;
    }
    return O;
}
function TypedArrayFilter(callbackfn) {
    var O = this;
    var isTypedArray = IsTypedArrayEnsuringArrayBuffer(O);
    var len;
    if (isTypedArray)
        len = TypedArrayLength(O);
    else
        len = callFunction(CallTypedArrayMethodIfWrapped, O, "TypedArrayLengthMethod");
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "%TypedArray%.prototype.filter");
    if (!IsCallable(callbackfn))
        ThrowTypeError(9, DecompileArg(0, callbackfn));
    var T = arguments.length > 1 ? arguments[1] : void 0;
    var kept = new List();
    var captured = 0;
    for (var k = 0; k < len; k++) {
        var kValue = O[k];
        var selected = ToBoolean(callContentFunction(callbackfn, T, kValue, k, O));
        if (selected) {
            kept[captured++] = kValue;
        }
    }
    var A = TypedArraySpeciesCreateWithLength(O, captured);
    for (var n = 0; n < captured; n++) {
        A[n] = kept[n];
    }
    return A;
}
function TypedArrayFind(predicate) {
    var O = this;
    var isTypedArray = IsTypedArrayEnsuringArrayBuffer(O);
    var len;
    if (isTypedArray)
        len = TypedArrayLength(O);
    else
        len = callFunction(CallTypedArrayMethodIfWrapped, O, "TypedArrayLengthMethod");
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "%TypedArray%.prototype.find");
    if (!IsCallable(predicate))
        ThrowTypeError(9, DecompileArg(0, predicate));
    var T = arguments.length > 1 ? arguments[1] : void 0;
    for (var k = 0; k < len; k++) {
        var kValue = O[k];
        if (callContentFunction(predicate, T, kValue, k, O))
            return kValue;
    }
    return undefined;
}
function TypedArrayFindIndex(predicate) {
    var O = this;
    var isTypedArray = IsTypedArrayEnsuringArrayBuffer(O);
    var len;
    if (isTypedArray)
        len = TypedArrayLength(O);
    else
        len = callFunction(CallTypedArrayMethodIfWrapped, O, "TypedArrayLengthMethod");
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "%TypedArray%.prototype.findIndex");
    if (!IsCallable(predicate))
        ThrowTypeError(9, DecompileArg(0, predicate));
    var T = arguments.length > 1 ? arguments[1] : void 0;
    for (var k = 0; k < len; k++) {
        if (callContentFunction(predicate, T, O[k], k, O))
            return k;
    }
    return -1;
}
function TypedArrayForEach(callbackfn) {
    var O = this;
    var isTypedArray = IsTypedArrayEnsuringArrayBuffer(O);
    var len;
    if (isTypedArray)
        len = TypedArrayLength(O);
    else
        len = callFunction(CallTypedArrayMethodIfWrapped, O, "TypedArrayLengthMethod");
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "TypedArray.prototype.forEach");
    if (!IsCallable(callbackfn))
        ThrowTypeError(9, DecompileArg(0, callbackfn));
    var T = arguments.length > 1 ? arguments[1] : void 0;
    for (var k = 0; k < len; k++) {
        callContentFunction(callbackfn, T, O[k], k, O);
    }
    return undefined;
}
function TypedArrayIndexOf(searchElement, fromIndex = 0) {
    if (!IsObject(this) || !IsTypedArray(this)) {
        return callFunction(CallTypedArrayMethodIfWrapped, this, searchElement, fromIndex,
                            "TypedArrayIndexOf");
    }
    GetAttachedArrayBuffer(this);
    var O = this;
    var len = TypedArrayLength(O);
    if (len === 0)
        return -1;
    var n = ToInteger(fromIndex);
    if (n >= len)
        return -1;
    var k;
    if (n >= 0) {
        k = n;
    } else {
        k = len + n;
        if (k < 0)
            k = 0;
    }
    for (; k < len; k++) {
        if (O[k] === searchElement)
            return k;
    }
    return -1;
}
function TypedArrayJoin(separator) {
    if (!IsObject(this) || !IsTypedArray(this)) {
        return callFunction(CallTypedArrayMethodIfWrapped, this, separator, "TypedArrayJoin");
    }
    GetAttachedArrayBuffer(this);
    var O = this;
    var len = TypedArrayLength(O);
    var sep = separator === undefined ? "," : ToString(separator);
    if (len === 0)
        return "";
    var element0 = O[0];
    var R = ToString(element0);
    for (var k = 1; k < len; k++) {
        var S = R + sep;
        var element = O[k];
        var next = ToString(element);
        R = S + next;
    }
    return R;
}
function TypedArrayKeys() {
    var O = this;
    IsTypedArrayEnsuringArrayBuffer(O);
    return CreateArrayIterator(O, 0);
}
function TypedArrayLastIndexOf(searchElement) {
    if (!IsObject(this) || !IsTypedArray(this)) {
        if (arguments.length > 1) {
            return callFunction(CallTypedArrayMethodIfWrapped, this, searchElement, arguments[1],
                                "TypedArrayLastIndexOf");
        }
        return callFunction(CallTypedArrayMethodIfWrapped, this, searchElement,
                            "TypedArrayLastIndexOf");
    }
    GetAttachedArrayBuffer(this);
    var O = this;
    var len = TypedArrayLength(O);
    if (len === 0)
        return -1;
    var n = arguments.length > 1 ? ToInteger(arguments[1]) : len - 1;
    var k = n >= 0 ? std_Math_min(n, len - 1) : len + n;
    for (; k >= 0; k--) {
        if (O[k] === searchElement)
            return k;
    }
    return -1;
}
function TypedArrayMap(callbackfn) {
    var O = this;
    var isTypedArray = IsTypedArrayEnsuringArrayBuffer(O);
    var len;
    if (isTypedArray)
        len = TypedArrayLength(O);
    else
        len = callFunction(CallTypedArrayMethodIfWrapped, O, "TypedArrayLengthMethod");
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "%TypedArray%.prototype.map");
    if (!IsCallable(callbackfn))
        ThrowTypeError(9, DecompileArg(0, callbackfn));
    var T = arguments.length > 1 ? arguments[1] : void 0;
    var A = TypedArraySpeciesCreateWithLength(O, len);
    for (var k = 0; k < len; k++) {
        var mappedValue = callContentFunction(callbackfn, T, O[k], k, O);
        A[k] = mappedValue;
    }
    return A;
}
function TypedArrayReduce(callbackfn) {
    var O = this;
    var isTypedArray = IsTypedArrayEnsuringArrayBuffer(O);
    var len;
    if (isTypedArray)
        len = TypedArrayLength(O);
    else
        len = callFunction(CallTypedArrayMethodIfWrapped, O, "TypedArrayLengthMethod");
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "%TypedArray%.prototype.reduce");
    if (!IsCallable(callbackfn))
        ThrowTypeError(9, DecompileArg(0, callbackfn));
    if (len === 0 && arguments.length === 1)
        ThrowTypeError(40);
    var k = 0;
    var accumulator = arguments.length > 1 ? arguments[1] : O[k++];
    for (; k < len; k++) {
        accumulator = callContentFunction(callbackfn, undefined, accumulator, O[k], k, O);
    }
    return accumulator;
}
function TypedArrayReduceRight(callbackfn) {
    var O = this;
    var isTypedArray = IsTypedArrayEnsuringArrayBuffer(O);
    var len;
    if (isTypedArray)
        len = TypedArrayLength(O);
    else
        len = callFunction(CallTypedArrayMethodIfWrapped, O, "TypedArrayLengthMethod");
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "%TypedArray%.prototype.reduceRight");
    if (!IsCallable(callbackfn))
        ThrowTypeError(9, DecompileArg(0, callbackfn));
    if (len === 0 && arguments.length === 1)
        ThrowTypeError(40);
    var k = len - 1;
    var accumulator = arguments.length > 1 ? arguments[1] : O[k--];
    for (; k >= 0; k--) {
        accumulator = callContentFunction(callbackfn, undefined, accumulator, O[k], k, O);
    }
    return accumulator;
}
function TypedArrayReverse() {
    if (!IsObject(this) || !IsTypedArray(this)) {
        return callFunction(CallTypedArrayMethodIfWrapped, this, "TypedArrayReverse");
    }
    GetAttachedArrayBuffer(this);
    var O = this;
    var len = TypedArrayLength(O);
    var middle = std_Math_floor(len / 2);
    for (var lower = 0; lower !== middle; lower++) {
        var upper = len - lower - 1;
        var lowerValue = O[lower];
        var upperValue = O[upper];
        O[lower] = upperValue;
        O[upper] = lowerValue;
    }
    return O;
}
function TypedArraySlice(start, end) {
    var O = this;
    if (!IsObject(O) || !IsTypedArray(O)) {
        return callFunction(CallTypedArrayMethodIfWrapped, O, start, end, "TypedArraySlice");
    }
    var buffer = GetAttachedArrayBuffer(O);
    var len = TypedArrayLength(O);
    var relativeStart = ToInteger(start);
    var k = relativeStart < 0
            ? std_Math_max(len + relativeStart, 0)
            : std_Math_min(relativeStart, len);
    var relativeEnd = end === undefined ? len : ToInteger(end);
    var final = relativeEnd < 0
                ? std_Math_max(len + relativeEnd, 0)
                : std_Math_min(relativeEnd, len);
    var count = std_Math_max(final - k, 0);
    var A = TypedArraySpeciesCreateWithLength(O, count);
    if (count > 0) {
        if (buffer === null) {
            buffer = ViewedArrayBufferIfReified(O);
        }
        if (IsDetachedBuffer(buffer))
            ThrowTypeError(509);
        var sliced = TypedArrayBitwiseSlice(O, A, k | 0, count | 0);
        if (!sliced) {
            var n = 0;
            while (k < final) {
                A[n++] = O[k++];
            }
        }
    }
    return A;
}
function TypedArraySome(callbackfn) {
    var O = this;
    var isTypedArray = IsTypedArrayEnsuringArrayBuffer(O);
    var len;
    if (isTypedArray)
        len = TypedArrayLength(O);
    else
        len = callFunction(CallTypedArrayMethodIfWrapped, O, "TypedArrayLengthMethod");
    if (arguments.length === 0)
        ThrowTypeError(42, 0, "%TypedArray%.prototype.some");
    if (!IsCallable(callbackfn))
        ThrowTypeError(9, DecompileArg(0, callbackfn));
    var T = arguments.length > 1 ? arguments[1] : void 0;
    for (var k = 0; k < len; k++) {
        var kValue = O[k];
        var testResult = callContentFunction(callbackfn, T, kValue, k, O);
        if (testResult)
            return true;
    }
    return false;
}
function TypedArrayCompare(x, y) {
    ;;
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    if (x === 0 && y === 0)
        return ((1 / x) > 0 ? 1 : 0) - ((1 / y) > 0 ? 1 : 0);
    if (Number_isNaN(x))
        return Number_isNaN(y) ? 0 : 1;
    return Number_isNaN(y) ? -1 : 0;
}
function TypedArrayCompareInt(x, y) {
    ;;
    ;;
    var diff = x - y;
    if (diff)
        return diff;
    return 0;
}
function TypedArrayCompareBigInt(x, y) {
    ;;
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}
function TypedArraySort(comparefn) {
    if (comparefn !== undefined) {
        if (!IsCallable(comparefn))
            ThrowTypeError(9, DecompileArg(0, comparefn));
    }
    var obj = this;
    var isTypedArray = IsObject(obj) && IsTypedArray(obj);
    var buffer;
    if (isTypedArray) {
        buffer = GetAttachedArrayBuffer(obj);
    } else {
        buffer = callFunction(CallTypedArrayMethodIfWrapped, obj, "GetAttachedArrayBufferMethod");
    }
    var len;
    if (isTypedArray) {
        len = TypedArrayLength(obj);
    } else {
        len = callFunction(CallTypedArrayMethodIfWrapped, obj, "TypedArrayLengthMethod");
    }
    if (len <= 1)
        return obj;
    if (comparefn === undefined) {
        var kind = GetTypedArrayKind(obj);
        switch (kind) {
          case 1:
          case 8:
            return CountingSort(obj, len, false , TypedArrayCompareInt);
          case 0:
            return CountingSort(obj, len, true , TypedArrayCompareInt);
          case 3:
            return RadixSort(obj, len, buffer,
                             2 , false , false ,
                             TypedArrayCompareInt);
          case 2:
            return RadixSort(obj, len, buffer,
                             2 , true , false ,
                             TypedArrayCompareInt);
          case 5:
            return RadixSort(obj, len, buffer,
                             4 , false , false ,
                             TypedArrayCompareInt);
          case 4:
            return RadixSort(obj, len, buffer,
                             4 , true , false ,
                             TypedArrayCompareInt);
          case 9:
          case 10:
            return QuickSort(obj, len, TypedArrayCompareBigInt);
          case 6:
            return RadixSort(obj, len, buffer,
                             4 , true , true ,
                             TypedArrayCompare);
          case 7:
          default:
            ;;
            return QuickSort(obj, len, TypedArrayCompare);
        }
    }
    var wrappedCompareFn = function(x, y) {
        var v = +comparefn(x, y);
        var length;
        if (isTypedArray) {
            length = TypedArrayLength(obj);
        } else {
            length = callFunction(CallTypedArrayMethodIfWrapped, obj, "TypedArrayLengthMethod");
        }
        if (length === 0) {
            ;;
            ThrowTypeError(509);
        }
        if (v !== v)
            return 0;
        return v;
    };
    return MergeSortTypedArray(obj, len, wrappedCompareFn);
}
function TypedArrayToLocaleString(locales = undefined, options = undefined) {
    var array = this;
    var isTypedArray = IsTypedArrayEnsuringArrayBuffer(array);
    var len;
    if (isTypedArray)
        len = TypedArrayLength(array);
    else
        len = callFunction(CallTypedArrayMethodIfWrapped, array, "TypedArrayLengthMethod");
    if (len === 0)
        return "";
    var firstElement = array[0];
    var R = ToString(callContentFunction(firstElement.toLocaleString, firstElement));
    var separator = ",";
    for (var k = 1; k < len; k++) {
        var S = R + separator;
        var nextElement = array[k];
        R = ToString(callContentFunction(nextElement.toLocaleString, nextElement));
        R = S + R;
    }
    return R;
}
function TypedArraySubarray(begin, end) {
    var obj = this;
    if (!IsObject(obj) || !IsTypedArray(obj)) {
        return callFunction(CallTypedArrayMethodIfWrapped, this, begin, end,
                            "TypedArraySubarray");
    }
    var buffer = ViewedArrayBufferIfReified(obj);
    if (buffer === null) {
        buffer = TypedArrayBuffer(obj);
    }
    var srcLength = TypedArrayLength(obj);
    var srcByteOffset = TypedArrayByteOffset(obj);
    var relativeBegin = ToInteger(begin);
    var beginIndex = relativeBegin < 0 ? std_Math_max(srcLength + relativeBegin, 0)
                                       : std_Math_min(relativeBegin, srcLength);
    var relativeEnd = end === undefined ? srcLength : ToInteger(end);
    var endIndex = relativeEnd < 0 ? std_Math_max(srcLength + relativeEnd, 0)
                                   : std_Math_min(relativeEnd, srcLength);
    var newLength = std_Math_max(endIndex - beginIndex, 0);
    var elementShift = TypedArrayElementShift(obj);
    var beginByteOffset = srcByteOffset + (beginIndex << elementShift);
    return TypedArraySpeciesCreateWithBuffer(obj, buffer, beginByteOffset, newLength);
}
function $TypedArrayValues() {
    var O = this;
    IsTypedArrayEnsuringArrayBuffer(O);
    return CreateArrayIterator(O, 1);
}
_SetCanonicalName($TypedArrayValues, "values");
function TypedArrayIncludes(searchElement, fromIndex = 0) {
    if (!IsObject(this) || !IsTypedArray(this)) {
        return callFunction(CallTypedArrayMethodIfWrapped, this, searchElement,
                            fromIndex, "TypedArrayIncludes");
    }
    GetAttachedArrayBuffer(this);
    var O = this;
    var len = TypedArrayLength(O);
    if (len === 0)
        return false;
    var n = ToInteger(fromIndex);
    var k;
    if (n >= 0) {
        k = n;
    } else {
        k = len + n;
        if (k < 0)
            k = 0;
    }
    while (k < len) {
        if (SameValueZero(searchElement, O[k]))
            return true;
        k++;
    }
    return false;
}
function TypedArrayStaticFrom(source, mapfn = undefined, thisArg = undefined) {
    var C = this;
    if (!IsConstructor(C))
        ThrowTypeError(10, typeof C);
    var mapping;
    if (mapfn !== undefined) {
        if (!IsCallable(mapfn))
            ThrowTypeError(9, DecompileArg(1, mapfn));
        mapping = true;
    } else {
        mapping = false;
    }
    var T = thisArg;
    var usingIterator = source[std_iterator];
    if (usingIterator !== undefined && usingIterator !== null) {
        if (!IsCallable(usingIterator))
            ThrowTypeError(60, DecompileArg(0, source));
        if (!mapping && IsTypedArrayConstructor(C) && IsObject(source)) {
            if (usingIterator === $TypedArrayValues && IsTypedArray(source) &&
                ArrayIteratorPrototypeOptimizable())
            {
                GetAttachedArrayBuffer(source);
                var len = TypedArrayLength(source);
                var targetObj = new C(len);
                for (var k = 0; k < len; k++) {
                    targetObj[k] = source[k];
                }
                return targetObj;
            }
            if (usingIterator === $ArrayValues && IsPackedArray(source) &&
                ArrayIteratorPrototypeOptimizable())
            {
                var targetObj = new C(source.length);
                TypedArrayInitFromPackedArray(targetObj, source);
                return targetObj;
            }
        }
        var values = IterableToList(source, usingIterator);
        var len = values.length;
        var targetObj = TypedArrayCreateWithLength(C, len);
        for (var k = 0; k < len; k++) {
            var kValue = values[k];
            var mappedValue = mapping ? callContentFunction(mapfn, T, kValue, k) : kValue;
            targetObj[k] = mappedValue;
        }
        return targetObj;
    }
    var arrayLike = ToObject(source);
    var len = ToLength(arrayLike.length);
    var targetObj = TypedArrayCreateWithLength(C, len);
    for (var k = 0; k < len; k++) {
        var kValue = arrayLike[k];
        var mappedValue = mapping ? callContentFunction(mapfn, T, kValue, k) : kValue;
        targetObj[k] = mappedValue;
    }
    return targetObj;
}
function TypedArrayStaticOf() {
    var len = arguments.length;
    var items = arguments;
    var C = this;
    if (!IsConstructor(C))
        ThrowTypeError(10, typeof C);
    var newObj = TypedArrayCreateWithLength(C, len);
    for (var k = 0; k < len; k++)
        newObj[k] = items[k];
    return newObj;
}
function $TypedArraySpecies() {
    return this;
}
_SetCanonicalName($TypedArraySpecies, "get [Symbol.species]");
function IterableToList(items, method) {
    ;;
    var iterator = callContentFunction(method, items);
    if (!IsObject(iterator))
        ThrowTypeError(62);
    var nextMethod = iterator.next;
    var values = [];
    var i = 0;
    while (true) {
        var next = callContentFunction(nextMethod, iterator);
        if (!IsObject(next))
            ThrowTypeError(63, "next");
        if (next.done)
            break;
        _DefineDataProperty(values, i++, next.value);
    }
    return values;
}
function ArrayBufferSlice(start, end) {
    var O = this;
    if (!IsObject(O) || (O = GuardToArrayBuffer(O)) === null) {
        return callFunction(CallArrayBufferMethodIfWrapped, this, start, end,
                            "ArrayBufferSlice");
    }
    if (IsDetachedBuffer(O))
        ThrowTypeError(509);
    var len = ArrayBufferByteLength(O);
    var relativeStart = ToInteger(start);
    var first = relativeStart < 0 ? std_Math_max(len + relativeStart, 0)
                                  : std_Math_min(relativeStart, len);
    var relativeEnd = end === undefined ? len : ToInteger(end);
    var final = relativeEnd < 0 ? std_Math_max(len + relativeEnd, 0)
                                : std_Math_min(relativeEnd, len);
    var newLen = std_Math_max(final - first, 0);
    var ctor = SpeciesConstructor(O, GetBuiltinConstructor("ArrayBuffer"));
    var new_ = new ctor(newLen);
    var isWrapped = false;
    var newBuffer;
    if ((newBuffer = GuardToArrayBuffer(new_)) !== null) {
        if (IsDetachedBuffer(newBuffer))
            ThrowTypeError(509);
    } else {
        newBuffer = new_;
        if (!IsWrappedArrayBuffer(newBuffer))
            ThrowTypeError(504);
        isWrapped = true;
        if (callFunction(CallArrayBufferMethodIfWrapped, newBuffer, "IsDetachedBufferThis"))
            ThrowTypeError(509);
    }
    if (newBuffer === O)
        ThrowTypeError(505);
    var actualLen = PossiblyWrappedArrayBufferByteLength(newBuffer);
    if (actualLen < newLen)
        ThrowTypeError(506, newLen, actualLen);
    if (IsDetachedBuffer(O))
        ThrowTypeError(509);
    ArrayBufferCopyData(newBuffer, 0, O, first | 0, newLen | 0, isWrapped);
    return newBuffer;
}
function IsDetachedBufferThis() {
  return IsDetachedBuffer(this);
}
function $ArrayBufferSpecies() {
    return this;
}
_SetCanonicalName($ArrayBufferSpecies, "get [Symbol.species]");
function $SharedArrayBufferSpecies() {
    return this;
}
_SetCanonicalName($SharedArrayBufferSpecies, "get [Symbol.species]");
function SharedArrayBufferSlice(start, end) {
    var O = this;
    if (!IsObject(O) || (O = GuardToSharedArrayBuffer(O)) === null) {
        return callFunction(CallSharedArrayBufferMethodIfWrapped, this, start, end,
                            "SharedArrayBufferSlice");
    }
    var len = SharedArrayBufferByteLength(O);
    var relativeStart = ToInteger(start);
    var first = relativeStart < 0 ? std_Math_max(len + relativeStart, 0)
                                  : std_Math_min(relativeStart, len);
    var relativeEnd = end === undefined ? len : ToInteger(end);
    var final = relativeEnd < 0 ? std_Math_max(len + relativeEnd, 0)
                                : std_Math_min(relativeEnd, len);
    var newLen = std_Math_max(final - first, 0);
    var ctor = SpeciesConstructor(O, GetBuiltinConstructor("SharedArrayBuffer"));
    var new_ = new ctor(newLen);
    var isWrapped = false;
    var newObj;
    if ((newObj = GuardToSharedArrayBuffer(new_)) === null) {
        if (!IsWrappedSharedArrayBuffer(new_))
            ThrowTypeError(516);
        isWrapped = true;
        newObj = new_;
    }
    if (newObj === O || SharedArrayBuffersMemorySame(newObj, O))
        ThrowTypeError(517);
    var actualLen = PossiblyWrappedSharedArrayBufferByteLength(newObj);
    if (actualLen < newLen)
        ThrowTypeError(518, newLen, actualLen);
    SharedArrayBufferCopyData(newObj, 0, O, first | 0, newLen | 0, isWrapped);
    return newObj;
}
function TypedObjectGet(descr, typedObj, offset) {
  ;;
  switch (UnsafeGetInt32FromReservedSlot(descr, 0)) {
  case 1:
    return TypedObjectGetScalar(descr, typedObj, offset);
  case 2:
    return TypedObjectGetReference(descr, typedObj, offset);
  case 4:
  case 3:
    return TypedObjectGetDerived(descr, typedObj, offset);
  }
  ;;
  return undefined;
}
function TypedObjectGetDerived(descr, typedObj, offset) {
  ;;
  return NewDerivedTypedObject(descr, typedObj, offset | 0);
}
function TypedObjectGetDerivedIf(descr, typedObj, offset, cond) {
  return (cond ? TypedObjectGetDerived(descr, typedObj, offset) : undefined);
}
function TypedObjectGetOpaque(descr, typedObj, offset) {
  ;;
  return NewOpaqueTypedObject(descr, typedObj, offset | 0);
}
function TypedObjectGetOpaqueIf(descr, typedObj, offset, cond) {
  return (cond ? TypedObjectGetOpaque(descr, typedObj, offset) : undefined);
}
function TypedObjectGetScalar(descr, typedObj, offset) {
  var type = UnsafeGetInt32FromReservedSlot(descr, 9);
  switch (type) {
  case 0:
    return Load_int8(typedObj, offset | 0);
  case 1:
  case 8:
    return Load_uint8(typedObj, offset | 0);
  case 2:
    return Load_int16(typedObj, offset | 0);
  case 3:
    return Load_uint16(typedObj, offset | 0);
  case 4:
    return Load_int32(typedObj, offset | 0);
  case 5:
    return Load_uint32(typedObj, offset | 0);
  case 6:
    return Load_float32(typedObj, offset | 0);
  case 7:
    return Load_float64(typedObj, offset | 0);
  case 9:
    return Load_bigint64(typedObj, offset | 0);
  case 10:
    return Load_biguint64(typedObj, offset | 0);
  }
  ;;
  return undefined;
}
function TypedObjectGetReference(descr, typedObj, offset) {
  var type = UnsafeGetInt32FromReservedSlot(descr, 9);
  switch (type) {
  case 0:
    return Load_Any(typedObj, offset | 0);
  case 1:
    return Load_Object(typedObj, offset | 0);
  case 2:
    var boxed = Load_WasmAnyRef(typedObj, offset | 0);
    if (!IsBoxedWasmAnyRef(boxed))
      return boxed;
    return UnboxBoxedWasmAnyRef(boxed);
  case 3:
    return Load_string(typedObj, offset | 0);
  }
  ;;
  return undefined;
}
function TypedObjectSet(descr, typedObj, offset, name, fromValue) {
  switch (UnsafeGetInt32FromReservedSlot(descr, 0)) {
  case 1:
    TypedObjectSetScalar(descr, typedObj, offset, fromValue);
    return;
  case 2:
    TypedObjectSetReference(descr, typedObj, offset, name, fromValue);
    return;
  case 4:
    var length = ((UnsafeGetInt32FromReservedSlot(descr, 10)) | 0);
    if (TypedObjectSetArray(descr, length, typedObj, offset, fromValue))
      return;
    break;
  case 3:
    if (!IsObject(fromValue))
      break;
    var fieldNames = UnsafeGetObjectFromReservedSlot(descr, 9);
    var fieldDescrs = UnsafeGetObjectFromReservedSlot(descr, 10);
    var fieldOffsets = UnsafeGetObjectFromReservedSlot(descr, 11);
    for (var i = 0; i < fieldNames.length; i++) {
      var fieldName = fieldNames[i];
      var fieldDescr = fieldDescrs[i];
      var fieldOffset = fieldOffsets[i];
      var fieldValue = fromValue[fieldName];
      TypedObjectSet(fieldDescr, typedObj, offset + fieldOffset, fieldName, fieldValue);
    }
    return;
  }
  ThrowTypeError(12,
                 typeof(fromValue),
                 UnsafeGetStringFromReservedSlot(descr, 1));
}
function TypedObjectSetArray(descr, length, typedObj, offset, fromValue) {
  if (!IsObject(fromValue))
    return false;
  if (fromValue.length !== length)
    return false;
  if (length > 0) {
    var elemDescr = UnsafeGetObjectFromReservedSlot(descr, 9);
    var elemSize = UnsafeGetInt32FromReservedSlot(elemDescr, 3);
    var elemOffset = offset;
    for (var i = 0; i < length; i++) {
      TypedObjectSet(elemDescr, typedObj, elemOffset, null, fromValue[i]);
      elemOffset += elemSize;
    }
  }
  return true;
}
function TypedObjectSetScalar(descr, typedObj, offset, fromValue) {
  ;;
  var type = UnsafeGetInt32FromReservedSlot(descr, 9);
  switch (type) {
  case 0:
    return Store_int8(typedObj, offset | 0,
                      ((fromValue) | 0) & 0xFF);
  case 1:
    return Store_uint8(typedObj, offset | 0,
                       ((fromValue) >>> 0) & 0xFF);
  case 8:
    var v = ClampToUint8(+fromValue);
    return Store_int8(typedObj, offset | 0, v);
  case 2:
    return Store_int16(typedObj, offset | 0,
                       ((fromValue) | 0) & 0xFFFF);
  case 3:
    return Store_uint16(typedObj, offset | 0,
                        ((fromValue) >>> 0) & 0xFFFF);
  case 4:
    return Store_int32(typedObj, offset | 0,
                       ((fromValue) | 0));
  case 5:
    return Store_uint32(typedObj, offset | 0,
                        ((fromValue) >>> 0));
  case 6:
    return Store_float32(typedObj, offset | 0, +fromValue);
  case 7:
    return Store_float64(typedObj, offset | 0, +fromValue);
  case 9:
    return Store_bigint64(typedObj, offset | 0, fromValue);
  case 10:
    return Store_biguint64(typedObj, offset | 0, fromValue);
  }
  ;;
  return undefined;
}
function TypedObjectSetReference(descr, typedObj, offset, name, fromValue) {
  var type = UnsafeGetInt32FromReservedSlot(descr, 9);
  switch (type) {
  case 0:
    return Store_Any(typedObj, offset | 0, name, fromValue);
  case 1:
    var value = (fromValue === null ? fromValue : ToObject(fromValue));
    return Store_Object(typedObj, offset | 0, name, value);
  case 2:
    var value = (IsBoxableWasmAnyRef(fromValue) ? BoxWasmAnyRef(fromValue) : fromValue);
    return Store_WasmAnyRef(typedObj, offset | 0, name, value);
  case 3:
    return Store_string(typedObj, offset | 0, name, ToString(fromValue));
  }
  ;;
  return undefined;
}
function ConvertAndCopyTo(destDescr,
                          destTypedObj,
                          destOffset,
                          fieldName,
                          fromValue)
{
  ;;
  ;;
  TypedObjectSet(destDescr, destTypedObj, destOffset, fieldName, fromValue);
}
function Reify(sourceDescr,
               sourceTypedObj,
               sourceOffset) {
  ;;
  ;;
  return TypedObjectGet(sourceDescr, sourceTypedObj, sourceOffset);
}
function TypeDescrEquivalent(otherDescr) {
  if (!IsObject(this) || !ObjectIsTypeDescr(this))
    ThrowTypeError(494);
  if (!IsObject(otherDescr) || !ObjectIsTypeDescr(otherDescr))
    ThrowTypeError(494);
  return DescrsEquiv(this, otherDescr);
}
function TypedObjectArrayRedimension(newArrayType) {
  if (!IsObject(this) || !ObjectIsTypedObject(this))
    ThrowTypeError(494);
  if (!IsObject(newArrayType) || !ObjectIsTypeDescr(newArrayType))
    ThrowTypeError(494);
  var oldArrayType = TypedObjectTypeDescr(this);
  var oldElementType = oldArrayType;
  var oldElementCount = 1;
  if (UnsafeGetInt32FromReservedSlot(oldArrayType, 0) != 4)
    ThrowTypeError(494);
  while (UnsafeGetInt32FromReservedSlot(oldElementType, 0) === 4) {
    oldElementCount *= oldElementType.length;
    oldElementType = oldElementType.elementType;
  }
  var newElementType = newArrayType;
  var newElementCount = 1;
  while (UnsafeGetInt32FromReservedSlot(newElementType, 0) == 4) {
    newElementCount *= newElementType.length;
    newElementType = newElementType.elementType;
  }
  if (oldElementCount !== newElementCount) {
    ThrowTypeError(494);
  }
  if (!DescrsEquiv(oldElementType, newElementType)) {
    ThrowTypeError(494);
  }
  ;;
  return NewDerivedTypedObject(newArrayType, this, 0);
}
function DescrsEquiv(descr1, descr2) {
  ;;
  ;;
  return UnsafeGetStringFromReservedSlot(descr1, 1) === UnsafeGetStringFromReservedSlot(descr2, 1);
}
function DescrToSource() {
  if (!IsObject(this) || !ObjectIsTypeDescr(this))
    ThrowTypeError(3, "Type", "toSource", "value");
  return UnsafeGetStringFromReservedSlot(this, 1);
}
function ArrayShorthand(...dims) {
  if (!IsObject(this) || !ObjectIsTypeDescr(this))
    ThrowTypeError(494);
  var AT = GetTypedObjectModule().ArrayType;
  if (dims.length == 0)
    ThrowTypeError(494);
  var accum = this;
  for (var i = dims.length - 1; i >= 0; i--)
    accum = new AT(accum, dims[i]);
  return accum;
}
function TypeOfTypedObject(obj) {
  if (IsObject(obj) && ObjectIsTypedObject(obj))
    return TypedObjectTypeDescr(obj);
  var T = GetTypedObjectModule();
  switch (typeof obj) {
    case "object": return T.Object;
    case "function": return T.Object;
    case "string": return T.String;
    case "number": return T.float64;
    case "undefined": return T.Any;
    default: return T.Any;
  }
}
function TypedObjectArrayTypeBuild(a, b, c) {
  if (!IsObject(this) || !ObjectIsTypeDescr(this))
    ThrowTypeError(494);
  var kind = UnsafeGetInt32FromReservedSlot(this, 0);
  switch (kind) {
  case 4:
    if (typeof a === "function") 
      return BuildTypedSeqImpl(this, this.length, 1, a);
    else if (typeof a === "number" && typeof b === "function")
      return BuildTypedSeqImpl(this, this.length, a, b);
    else if (typeof a === "number")
      ThrowTypeError(494);
    else
      ThrowTypeError(494);
      break;
  default:
    ThrowTypeError(494);
  }
}
function TypedObjectArrayTypeFrom(a, b, c) {
  if (!IsObject(this) || !ObjectIsTypeDescr(this))
    ThrowTypeError(494);
  var untypedInput = !IsObject(a) || !ObjectIsTypedObject(a) ||
                     !TypeDescrIsArrayType(TypedObjectTypeDescr(a));
  if (untypedInput) {
    if (b === 1 && IsCallable(c))
      return MapUntypedSeqImpl(a, this, c);
    if (IsCallable(b))
      return MapUntypedSeqImpl(a, this, b);
    ThrowTypeError(494);
  }
  if (typeof b === "number" && IsCallable(c))
    return MapTypedSeqImpl(a, b, this, c);
  if (IsCallable(b))
    return MapTypedSeqImpl(a, 1, this, b);
  ThrowTypeError(494);
}
function TypedObjectArrayMap(a, b) {
  if (!IsObject(this) || !ObjectIsTypedObject(this))
    ThrowTypeError(494);
  var thisType = TypedObjectTypeDescr(this);
  if (!TypeDescrIsArrayType(thisType))
    ThrowTypeError(494);
  if (typeof a === "number" && typeof b === "function")
    return MapTypedSeqImpl(this, a, thisType, b);
  else if (typeof a === "function")
    return MapTypedSeqImpl(this, 1, thisType, a);
  ThrowTypeError(494);
}
function TypedObjectArrayReduce(a, b) {
  if (!IsObject(this) || !ObjectIsTypedObject(this))
    ThrowTypeError(494);
  var thisType = TypedObjectTypeDescr(this);
  if (!TypeDescrIsArrayType(thisType))
    ThrowTypeError(494);
  if (a !== undefined && typeof a !== "function")
    ThrowTypeError(494);
  var outputType = thisType.elementType;
  return ReduceTypedSeqImpl(this, outputType, a, b);
}
function TypedObjectArrayFilter(func) {
  if (!IsObject(this) || !ObjectIsTypedObject(this))
    ThrowTypeError(494);
  var thisType = TypedObjectTypeDescr(this);
  if (!TypeDescrIsArrayType(thisType))
    ThrowTypeError(494);
  if (typeof func !== "function")
    ThrowTypeError(494);
  return FilterTypedSeqImpl(this, func);
}
function NUM_BYTES(bits) {
  return (bits + 7) >> 3;
}
function SET_BIT(data, index) {
  var word = index >> 3;
  var mask = 1 << (index & 0x7);
  data[word] |= mask;
}
function GET_BIT(data, index) {
  var word = index >> 3;
  var mask = 1 << (index & 0x7);
  return (data[word] & mask) != 0;
}
function BuildTypedSeqImpl(arrayType, len, depth, func) {
  ;;
  if (depth <= 0 || ((depth) | 0) !== depth) {
    ThrowTypeError(494);
  }
  var {iterationSpace, grainType, totalLength} =
    ComputeIterationSpace(arrayType, depth, len);
  var result = new arrayType();
  var indices = new List();
  indices.length = depth;
  for (var i = 0; i < depth; i++) {
    indices[i] = 0;
  }
  var grainTypeIsComplex = !TypeDescrIsSimpleType(grainType);
  var size = UnsafeGetInt32FromReservedSlot(grainType, 3);
  var outOffset = 0;
  for (i = 0; i < totalLength; i++) {
    var userOutPointer = TypedObjectGetOpaqueIf(grainType, result, outOffset,
                                                grainTypeIsComplex);
    callFunction(std_Array_push, indices, userOutPointer);
    var r = callFunction(std_Function_apply, func, undefined, indices);
    callFunction(std_Array_pop, indices);
    if (r !== undefined)
      TypedObjectSet(grainType, result, outOffset, null, r); 
    IncrementIterationSpace(indices, iterationSpace);
    outOffset += size;
  }
  return result;
}
function ComputeIterationSpace(arrayType, depth, len) {
  ;;
  ;;
  ;;
  var iterationSpace = new List();
  iterationSpace.length = depth;
  iterationSpace[0] = len;
  var totalLength = len;
  var grainType = arrayType.elementType;
  for (var i = 1; i < depth; i++) {
    if (TypeDescrIsArrayType(grainType)) {
      var grainLen = grainType.length;
      iterationSpace[i] = grainLen;
      totalLength *= grainLen;
      grainType = grainType.elementType;
    } else {
      ThrowTypeError(494);
    }
  }
  return { iterationSpace, grainType, totalLength };
}
function IncrementIterationSpace(indices, iterationSpace) {
  ;;
  var n = indices.length - 1;
  while (true) {
    indices[n] += 1;
    if (indices[n] < iterationSpace[n])
      return;
    ;;
    indices[n] = 0;
    if (n == 0)
      return;
    n -= 1;
  }
}
function MapUntypedSeqImpl(inArray, outputType, maybeFunc) {
  ;;
  ;;
  inArray = ToObject(inArray);
  ;;
  if (!IsCallable(maybeFunc))
    ThrowTypeError(9, DecompileArg(0, maybeFunc));
  var func = maybeFunc;
  var outLength = outputType.length;
  var outGrainType = outputType.elementType;
  var result = new outputType();
  var outUnitSize = UnsafeGetInt32FromReservedSlot(outGrainType, 3);
  var outGrainTypeIsComplex = !TypeDescrIsSimpleType(outGrainType);
  var outOffset = 0;
  for (var i = 0; i < outLength; i++) {
    if (i in inArray) { 
      var element = inArray[i];
      var out = TypedObjectGetOpaqueIf(outGrainType, result, outOffset,
                                       outGrainTypeIsComplex);
      var r = func(element, i, inArray, out);
      if (r !== undefined)
        TypedObjectSet(outGrainType, result, outOffset, null, r); 
    }
    outOffset += outUnitSize;
  }
  return result;
}
function MapTypedSeqImpl(inArray, depth, outputType, func) {
  ;;
  ;;
  ;;
  if (depth <= 0 || ((depth) | 0) !== depth)
    ThrowTypeError(494);
  var inputType = TypeOfTypedObject(inArray);
  var {iterationSpace: inIterationSpace, grainType: inGrainType} =
    ComputeIterationSpace(inputType, depth, inArray.length);
  if (!IsObject(inGrainType) || !ObjectIsTypeDescr(inGrainType))
    ThrowTypeError(494);
  var {iterationSpace, grainType: outGrainType, totalLength} =
    ComputeIterationSpace(outputType, depth, outputType.length);
  for (var i = 0; i < depth; i++)
    if (inIterationSpace[i] !== iterationSpace[i])
      ThrowTypeError(494);
  var result = new outputType();
  var inGrainTypeIsComplex = !TypeDescrIsSimpleType(inGrainType);
  var outGrainTypeIsComplex = !TypeDescrIsSimpleType(outGrainType);
  var inOffset = 0;
  var outOffset = 0;
  var isDepth1Simple = depth == 1 && !(inGrainTypeIsComplex || outGrainTypeIsComplex);
  var inUnitSize = isDepth1Simple ? 0 : UnsafeGetInt32FromReservedSlot(inGrainType, 3);
  var outUnitSize = isDepth1Simple ? 0 : UnsafeGetInt32FromReservedSlot(outGrainType, 3);
  function DoMapTypedSeqDepth1() {
    for (var i = 0; i < totalLength; i++) {
      var element = TypedObjectGet(inGrainType, inArray, inOffset);
      var out = TypedObjectGetOpaqueIf(outGrainType, result, outOffset,
                                       outGrainTypeIsComplex);
      var r = func(element, i, inArray, out);
      if (r !== undefined)
        TypedObjectSet(outGrainType, result, outOffset, null, r); 
      inOffset += inUnitSize;
      outOffset += outUnitSize;
    }
    return result;
  }
  function DoMapTypedSeqDepth1Simple(inArray, totalLength, func, result) {
    for (var i = 0; i < totalLength; i++) {
      var r = func(inArray[i], i, inArray, undefined);
      if (r !== undefined)
        result[i] = r;
    }
    return result;
  }
  function DoMapTypedSeqDepthN() {
    var indices = new List();
    for (var i = 0; i < depth; i++)
        callFunction(std_Array_push, indices, 0);
    for (var i = 0; i < totalLength; i++) {
      var element = TypedObjectGet(inGrainType, inArray, inOffset);
      var out = TypedObjectGetOpaqueIf(outGrainType, result, outOffset,
                                       outGrainTypeIsComplex);
      var args = [element];
      callFunction(std_Function_apply, std_Array_push, args, indices);
      callFunction(std_Array_push, args, inArray, out);
      var r = callFunction(std_Function_apply, func, void 0, args);
      if (r !== undefined)
        TypedObjectSet(outGrainType, result, outOffset, null, r); 
      inOffset += inUnitSize;
      outOffset += outUnitSize;
      IncrementIterationSpace(indices, iterationSpace);
    }
    return result;
  }
  if (isDepth1Simple)
    return DoMapTypedSeqDepth1Simple(inArray, totalLength, func, result);
  if (depth == 1)
    return DoMapTypedSeqDepth1();
  return DoMapTypedSeqDepthN();
}
function ReduceTypedSeqImpl(array, outputType, func, initial) {
  ;;
  ;;
  var start, value;
  if (initial === undefined && array.length < 1)
    ThrowTypeError(494);
  if (TypeDescrIsSimpleType(outputType)) {
    if (initial === undefined) {
      start = 1;
      value = array[0];
    } else {
      start = 0;
      value = outputType(initial);
    }
    for (var i = start; i < array.length; i++)
      value = outputType(func(value, array[i]));
  } else {
    if (initial === undefined) {
      start = 1;
      value = new outputType(array[0]);
    } else {
      start = 0;
      value = initial;
    }
    for (var i = start; i < array.length; i++)
      value = func(value, array[i]);
  }
  return value;
}
function FilterTypedSeqImpl(array, func) {
  ;;
  ;;
  var arrayType = TypeOfTypedObject(array);
  if (!TypeDescrIsArrayType(arrayType))
    ThrowTypeError(494);
  var elementType = arrayType.elementType;
  var flags = new Uint8Array(NUM_BYTES(array.length));
  var count = 0;
  var size = UnsafeGetInt32FromReservedSlot(elementType, 3);
  var inOffset = 0;
  for (var i = 0; i < array.length; i++) {
    var v = TypedObjectGet(elementType, array, inOffset);
    if (func(v, i, array)) {
      SET_BIT(flags, i);
      count++;
    }
    inOffset += size;
  }
  var AT = GetTypedObjectModule().ArrayType;
  var resultType = new AT(elementType, count);
  var result = new resultType();
  for (var i = 0, j = 0; i < array.length; i++) {
    if (GET_BIT(flags, i))
      result[j++] = array[i];
  }
  return result;
}
function WeakMapConstructorInit(iterable) {
    var map = this;
    var adder = map.set;
    if (!IsCallable(adder))
        ThrowTypeError(9, typeof adder);
    for (var nextItem of allowContentIter(iterable)) {
        if (!IsObject(nextItem))
            ThrowTypeError(33, "WeakMap");
        callContentFunction(adder, map, nextItem[0], nextItem[1]);
    }
}
function WeakSetConstructorInit(iterable) {
    var set = this;
    var adder = set.add;
    if (!IsCallable(adder))
        ThrowTypeError(9, typeof adder);
    for (var nextValue of allowContentIter(iterable))
        callContentFunction(adder, set, nextValue);
}
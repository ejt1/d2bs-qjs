/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jit_MOpcodesGenerated_h
#define jit_MOpcodesGenerated_h

/* This file is generated by jit/GenerateOpcodeFiles.py. Do not edit! */

#define MIR_OPCODE_LIST(_) \
_(Start)\
_(OsrEntry)\
_(Nop)\
_(LimitedTruncate)\
_(Constant)\
_(WasmNullConstant)\
_(WasmFloatConstant)\
_(CloneLiteral)\
_(Parameter)\
_(Callee)\
_(IsConstructing)\
_(TableSwitch)\
_(Goto)\
_(Test)\
_(GotoWithFake)\
_(Return)\
_(Throw)\
_(NewArray)\
_(NewArrayCopyOnWrite)\
_(NewArrayDynamicLength)\
_(NewTypedArray)\
_(NewTypedArrayDynamicLength)\
_(NewTypedArrayFromArray)\
_(NewTypedArrayFromArrayBuffer)\
_(NewObject)\
_(NewIterator)\
_(ObjectState)\
_(ArrayState)\
_(ArgumentState)\
_(MutateProto)\
_(InitPropGetterSetter)\
_(InitElem)\
_(InitElemGetterSetter)\
_(Call)\
_(ApplyArgs)\
_(ApplyArray)\
_(ConstructArray)\
_(Bail)\
_(Unreachable)\
_(EncodeSnapshot)\
_(AssertRecoveredOnBailout)\
_(AssertFloat32)\
_(GetDynamicName)\
_(CallDirectEval)\
_(Compare)\
_(SameValue)\
_(Box)\
_(Unbox)\
_(GuardObject)\
_(GuardString)\
_(PolyInlineGuard)\
_(AssertRange)\
_(CreateThisWithTemplate)\
_(CreateThisWithProto)\
_(CreateThis)\
_(CreateArgumentsObject)\
_(GetArgumentsObjectArg)\
_(SetArgumentsObjectArg)\
_(ReturnFromCtor)\
_(ToDouble)\
_(ToFloat32)\
_(WasmUnsignedToDouble)\
_(WasmUnsignedToFloat32)\
_(WrapInt64ToInt32)\
_(ExtendInt32ToInt64)\
_(WasmTruncateToInt64)\
_(WasmTruncateToInt32)\
_(WasmBoxValue)\
_(WasmAnyRefFromJSObject)\
_(Int64ToFloatingPoint)\
_(ToNumberInt32)\
_(ToIntegerInt32)\
_(TruncateToInt32)\
_(ToBigInt)\
_(ToInt64)\
_(TruncateBigIntToInt64)\
_(Int64ToBigInt)\
_(ToString)\
_(ToObject)\
_(ToObjectOrNull)\
_(BitNot)\
_(TypeOf)\
_(ToAsyncIter)\
_(ToId)\
_(BitAnd)\
_(BitOr)\
_(BitXor)\
_(Lsh)\
_(Rsh)\
_(Ursh)\
_(SignExtendInt32)\
_(SignExtendInt64)\
_(MinMax)\
_(Abs)\
_(Clz)\
_(Ctz)\
_(Popcnt)\
_(Sqrt)\
_(CopySign)\
_(Atan2)\
_(Hypot)\
_(Pow)\
_(PowHalf)\
_(Random)\
_(Sign)\
_(MathFunction)\
_(Add)\
_(Sub)\
_(Mul)\
_(Div)\
_(Mod)\
_(Concat)\
_(CharCodeAt)\
_(FromCharCode)\
_(FromCodePoint)\
_(StringConvertCase)\
_(StringSplit)\
_(ComputeThis)\
_(ImplicitThis)\
_(ArrowNewTarget)\
_(Phi)\
_(Beta)\
_(NaNToZero)\
_(OsrValue)\
_(OsrEnvironmentChain)\
_(OsrArgumentsObject)\
_(OsrReturnValue)\
_(BinaryCache)\
_(UnaryCache)\
_(CheckOverRecursed)\
_(InterruptCheck)\
_(WasmInterruptCheck)\
_(WasmTrap)\
_(LexicalCheck)\
_(ThrowRuntimeLexicalError)\
_(GlobalNameConflictsCheck)\
_(DefVar)\
_(DefLexical)\
_(DefFun)\
_(RegExp)\
_(RegExpMatcher)\
_(RegExpSearcher)\
_(RegExpTester)\
_(RegExpPrototypeOptimizable)\
_(RegExpInstanceOptimizable)\
_(GetFirstDollarIndex)\
_(StringReplace)\
_(Substr)\
_(ClassConstructor)\
_(DerivedClassConstructor)\
_(ModuleMetadata)\
_(DynamicImport)\
_(Lambda)\
_(LambdaArrow)\
_(FunctionWithProto)\
_(SetFunName)\
_(Slots)\
_(Elements)\
_(ConstantElements)\
_(ConvertElementsToDoubles)\
_(MaybeToDoubleElement)\
_(MaybeCopyElementsForWrite)\
_(InitializedLength)\
_(SetInitializedLength)\
_(ArrayLength)\
_(SetArrayLength)\
_(GetNextEntryForIterator)\
_(ArrayBufferViewLength)\
_(ArrayBufferViewByteOffset)\
_(ArrayBufferViewElements)\
_(TypedArrayElementShift)\
_(TypedArrayIndexToInt32)\
_(KeepAliveObject)\
_(Not)\
_(BoundsCheck)\
_(BoundsCheckLower)\
_(SpectreMaskIndex)\
_(LoadElement)\
_(LoadElementAndUnbox)\
_(LoadElementHole)\
_(LoadElementFromState)\
_(StoreElement)\
_(StoreElementHole)\
_(FallibleStoreElement)\
_(ArrayPopShift)\
_(ArrayPush)\
_(ArraySlice)\
_(ArrayJoin)\
_(LoadUnboxedScalar)\
_(LoadDataViewElement)\
_(LoadTypedArrayElementHole)\
_(StoreUnboxedScalar)\
_(StoreDataViewElement)\
_(StoreTypedArrayElementHole)\
_(EffectiveAddress)\
_(ClampToUint8)\
_(LoadFixedSlot)\
_(LoadFixedSlotAndUnbox)\
_(LoadDynamicSlotAndUnbox)\
_(StoreFixedSlot)\
_(GetPropertyCache)\
_(HomeObjectSuperBase)\
_(GetPropSuperCache)\
_(GetPropertyPolymorphic)\
_(SetPropertyPolymorphic)\
_(ObjectGroupDispatch)\
_(FunctionDispatch)\
_(BindNameCache)\
_(CallBindVar)\
_(GuardShape)\
_(GuardValue)\
_(GuardNullOrUndefined)\
_(GuardReceiverPolymorphic)\
_(GuardObjectGroup)\
_(GuardObjectIdentity)\
_(GuardSpecificFunction)\
_(GuardSpecificAtom)\
_(LoadDynamicSlot)\
_(FunctionEnvironment)\
_(NewLexicalEnvironmentObject)\
_(CopyLexicalEnvironmentObject)\
_(HomeObject)\
_(StoreDynamicSlot)\
_(GetNameCache)\
_(CallGetIntrinsicValue)\
_(DeleteProperty)\
_(DeleteElement)\
_(CallSetProperty)\
_(SetPropertyCache)\
_(CallGetProperty)\
_(CallGetElement)\
_(CallSetElement)\
_(CallInitElementArray)\
_(SetDOMProperty)\
_(GetDOMProperty)\
_(GetDOMMember)\
_(StringLength)\
_(Floor)\
_(Ceil)\
_(Round)\
_(Trunc)\
_(NearbyInt)\
_(GetIteratorCache)\
_(IteratorMore)\
_(IsNoIter)\
_(IteratorEnd)\
_(InCache)\
_(InArray)\
_(HasOwnCache)\
_(InstanceOf)\
_(InstanceOfCache)\
_(ArgumentsLength)\
_(GetFrameArgument)\
_(NewTarget)\
_(Rest)\
_(FilterTypeSet)\
_(TypeBarrier)\
_(PostWriteBarrier)\
_(PostWriteElementBarrier)\
_(NewNamedLambdaObject)\
_(NewCallObject)\
_(NewStringObject)\
_(IsCallable)\
_(IsConstructor)\
_(IsObject)\
_(IsNullOrUndefined)\
_(HasClass)\
_(GuardToClass)\
_(IsArray)\
_(IsTypedArray)\
_(ObjectClassToString)\
_(CheckReturn)\
_(CheckThis)\
_(CheckThisReinit)\
_(RecompileCheck)\
_(AtomicIsLockFree)\
_(GuardSharedTypedArray)\
_(CompareExchangeTypedArrayElement)\
_(AtomicExchangeTypedArrayElement)\
_(AtomicTypedArrayElementBinop)\
_(Debugger)\
_(CheckIsObj)\
_(CheckObjCoercible)\
_(CheckClassHeritage)\
_(DebugCheckSelfHosted)\
_(FinishBoundFunctionInit)\
_(IsPackedArray)\
_(GetPrototypeOf)\
_(ObjectWithProto)\
_(ObjectStaticProto)\
_(FunctionProto)\
_(SuperFunction)\
_(InitHomeObject)\
_(WasmNeg)\
_(WasmLoadTls)\
_(WasmHeapBase)\
_(WasmBoundsCheck)\
_(WasmAddOffset)\
_(WasmAlignmentCheck)\
_(WasmLoad)\
_(WasmStore)\
_(AsmJSLoadHeap)\
_(AsmJSStoreHeap)\
_(WasmFence)\
_(WasmCompareExchangeHeap)\
_(WasmAtomicExchangeHeap)\
_(WasmAtomicBinopHeap)\
_(WasmLoadGlobalVar)\
_(WasmLoadGlobalCell)\
_(WasmStoreGlobalVar)\
_(WasmStoreGlobalCell)\
_(WasmStoreStackResult)\
_(WasmDerivedPointer)\
_(WasmStoreRef)\
_(WasmParameter)\
_(WasmReturn)\
_(WasmReturnVoid)\
_(WasmStackArg)\
_(WasmRegisterResult)\
_(WasmFloatRegisterResult)\
_(WasmRegister64Result)\
_(WasmStackResultArea)\
_(WasmStackResult)\
_(WasmCall)\
_(WasmSelect)\
_(WasmReinterpret)\
_(Rotate)\
_(WasmBitselectSimd128)\
_(WasmBinarySimd128)\
_(WasmShiftSimd128)\
_(WasmShuffleSimd128)\
_(WasmReplaceLaneSimd128)\
_(WasmUnarySimd128)\
_(WasmScalarToSimd128)\
_(WasmReduceSimd128)\
_(UnknownValue)\
_(IonToWasmCall)

#endif // jit_MOpcodesGenerated_h
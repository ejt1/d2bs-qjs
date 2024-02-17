//#include "JSSocket.h"
//
//#include "Bindings.h"
//
//JSValue SocketWrap::Instantiate(JSContext* ctx, JSValue new_target) {
//  JSValue proto;
//  if (JS_IsUndefined(new_target)) {
//    proto = JS_GetClassProto(ctx, m_class_id);
//  } else {
//    proto = JS_GetPropertyStr(ctx, new_target, "prototype");
//    if (JS_IsException(proto)) {
//      return JS_EXCEPTION;
//    }
//  }
//  JSValue obj = JS_NewObjectProtoClass(ctx, proto, m_class_id);
//  JS_FreeValue(ctx, proto);
//  if (JS_IsException(obj)) {
//    return obj;
//  }
//
//  SocketWrap* wrap = new SocketWrap(ctx);
//  if (!wrap) {
//    JS_FreeValue(ctx, obj);
//    return JS_ThrowOutOfMemory(ctx);
//  }
//  JS_SetOpaque(obj, wrap);
//
//  return obj;
//}
//
//void SocketWrap::Initialize(JSContext* ctx, JSValue target) {
//  JSClassDef def{};
//  def.class_name = "Socket";
//  def.finalizer = [](JSRuntime* /*rt*/, JSValue val) {
//    SocketWrap* wrap = static_cast<SocketWrap*>(JS_GetOpaque(val, m_class_id));
//    if (wrap) {
//      delete wrap;
//    }
//  };
//
//  if (m_class_id == 0) {
//    JS_NewClassID(&m_class_id);
//  }
//  JS_NewClass(JS_GetRuntime(ctx), m_class_id, &def);
//
//  JSValue proto = JS_NewObject(ctx);
//  JS_SetPropertyFunctionList(ctx, proto, m_proto_funcs, _countof(m_proto_funcs));
//
//  JSValue obj = JS_NewObjectProtoClass(ctx, proto, m_class_id);
//  JS_SetPropertyFunctionList(ctx, obj, m_static_funcs, _countof(m_static_funcs));
//  JS_SetClassProto(ctx, m_class_id, proto);
//  JS_SetPropertyStr(ctx, target, "Socket", obj);
//}
//
//SocketWrap::SocketWrap(JSContext* /*ctx*/) {
//}
//
//// properties
//JSValue SocketWrap::GetReadable(JSContext* ctx, JSValue /*this_val*/) {
//  return JS_ThrowInternalError(ctx, "unimplemented");
//  //      fd_set read_set;
//  //      timeout.tv_sec = 0;
//  //      timeout.tv_usec = 100;  // 100 ms
//  //      FD_ZERO(&read_set);
//  //      FD_SET(sdata->socket, &read_set);
//
//  //      result = select(1, &read_set, NULL, NULL, &timeout);
//  //      vp.setInt32(result);
//}
//
//JSValue SocketWrap::GetWriteable(JSContext* ctx, JSValue /*this_val*/) {
//  return JS_ThrowInternalError(ctx, "unimplemented");
//  //      fd_set write_set;
//
//  //      timeout.tv_sec = 0;
//  //      timeout.tv_usec = 100;  // 100 ms
//  //      FD_ZERO(&write_set);
//  //      FD_SET(sdata->socket, &write_set);
//
//  //      result = select(1, NULL, &write_set, NULL, &timeout);
//
//  //      vp.setInt32(result);
//}
//
//// functions
//JSValue SocketWrap::Close(JSContext* ctx, JSValue /*this_val*/, int /*argc*/, JSValue* /*argv*/) {
//  return JS_ThrowInternalError(ctx, "unimplemented");
//  //(argc);
//
//  // SocketData* sData = (SocketData*)JS_GetOpaque3(this_val);
//
//  // closesocket(sData->socket);
//  // WSACleanup();
//}
//
//JSValue SocketWrap::Send(JSContext* ctx, JSValue /*this_val*/, int /*argc*/, JSValue* /*argv*/) {
//  return JS_ThrowInternalError(ctx, "unimplemented");
//  //(argc);
//
//  // SocketData* sData = (SocketData*)JS_GetOpaque3(this_val);
//  // char* msg = NULL;
//
//  // if (JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
//  //   msg = JS_EncodeStringToUTF8(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
//
//  // send(sData->socket, msg, strlen(msg), 0);
//}
//
//JSValue SocketWrap::Read(JSContext* ctx, JSValue /*this_val*/, int /*argc*/, JSValue* /*argv*/) {
//  return JS_ThrowInternalError(ctx, "unimplemented");
//  //(argc);
//
//  // SocketData* sData = (SocketData*)JS_GetOpaque3(this_val);
//
//  // char buffer[10000] = {0};
//  // std::string returnVal;
//  ////int nDataLength;
//
//  // int iResult = 0;
//  // do {
//  //   iResult = recv(sData->socket, buffer, 10000, 0);
//  //   if (iResult > 0)
//  //     returnVal.append(buffer);
//  //   if (iResult == -1) {
//  //     THROW_ERROR(cx, "Socket Error");
//  //   }
//
//  //} while (iResult > 10000);
//
//  // wchar_t* wret = AnsiToUnicode(returnVal.c_str());
//  // JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(JS_InternUCString(cx, wret)));
//  // delete[] wret;
//}
//
//// static functions
//JSValue SocketWrap::Open(JSContext* ctx, JSValue /*this_val*/, int /*argc*/, JSValue* /*argv*/) {
//  return JS_ThrowInternalError(ctx, "unimplemented");
//  // if (argc < 2) {
//  //   JS_SET_RVAL(cx, vp, JSVAL_FALSE);
//  //   return JS_TRUE;
//  // }
//  // char* hostName = NULL;
//  // int32_t port = 0;
//  // if (JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
//  //   hostName = JS_EncodeStringToUTF8(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
//
//  // if (!strstr(Vars.szHosts, hostName))
//  //   THROW_ERROR(cx, "Invalid hostname specified");
//
//  // if (JS_ValueToInt32(cx, JS_ARGV(cx, vp)[1], &port) == JS_FALSE)
//  //   THROW_ERROR(cx, "Could not convert parameter 2");
//
//  // WSADATA wsaData;
//
//  // if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
//  //   return JS_TRUE;
//  // }
//
//  // SocketData* Sdata = new SocketData;
//
//  // Sdata->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//
//  // struct hostent* host;
//  // host = gethostbyname(hostName);
//  // JS_free(cx, hostName);
//  // if (host == NULL)
//  //   THROW_ERROR(cx, "Cannot find host");
//  // SOCKADDR_IN SockAddr;
//  // SockAddr.sin_port = htons(static_cast<u_short>(port));
//  // SockAddr.sin_family = AF_INET;
//  // SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);
//  // Sdata->mode = Sdata->socket;
//
//  // if (connect(Sdata->socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0) {
//  //   THROW_ERROR(cx, "Failed to connect");
//  // }
//
//  // JSObject* res = BuildObject(cx, &socket_class, socket_methods, socket_props, Sdata);
//  // if (!res) {
//  //   closesocket(Sdata->socket);
//  //   WSACleanup();
//  //   THROW_ERROR(cx, "Failed to define the socket object");
//  // }
//  // JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(res));
//}
//
//D2BS_BINDING_INTERNAL(SocketWrap, SocketWrap::Initialize)

#ifndef NODE_OPENCL_TYPES_H_
#define NODE_OPENCL_TYPES_H_


#include <node.h>
#include "nan.h"
#include <string>
#include <memory>
#include <vector>
#include <v8.h>

#if defined (__APPLE__) || defined(MACOSX)
#ifdef __ECLIPSE__
    #include <cl_platform.h>
    #include <cl.h>
    #include <cl_ext.h>
  #else
#include <OpenCL/opencl.h>
#endif
#define HAS_clGetContextInfo
#elif defined(_WIN32)
    #include <CL/opencl.h>
    #define strcasecmp _stricmp
    #define strncasecmp _strnicmp
    char *strcasestr(const char *s, char *find);
#else
    #include <CL/opencl.h>
#endif

using namespace std;
using namespace v8;

template <typename T>
T * NoCLUnwrap(Local<Value> val) {
  if (val->IsNull() || val->IsUndefined()) {
    return NULL;
  }

  if (!val->IsObject() || val->IsArray()) {
    return NULL;
  }

  Local<Object> obj = val->ToObject();

  if (NanGetInternalFieldPointer(obj, 0) == NULL) {
    return NULL;
  }

  if (NanGetInternalFieldPointer(obj, 1) == NULL) {
    return NULL;
  }

  unsigned int identifier = * (unsigned int *) NanGetInternalFieldPointer(obj, 1);

  if (identifier != T::GetId()) {
    return NULL;
  }


  T * output = (T *) NanGetInternalFieldPointer(obj, 0);
  return output;
}

Local<ObjectTemplate> & GetNodeOpenCLObjectGenericTemplate();

#define NOCL_UNWRAP(VAR, TYPE, EXPR) \
  TYPE * VAR = NoCLUnwrap<TYPE>(EXPR);\
  if (VAR == NULL) { NanThrowError(opencl::getExceptionMessage(TYPE::getErrorCode()).c_str(), TYPE::getErrorCode()); NanReturnUndefined(); }


class NoCLObjectGen {

protected:
  const void * raw;

public:
  const void * getUncastedRaw() {
    return raw;
  }

};

template <typename T, unsigned int elid, int err>
class NoCLObject : public NoCLObjectGen {
public:

  NoCLObject(T raw) {
    this->raw = raw;
  }

  const T getRaw() {
    return (T) this->raw;
  }

  static unsigned int GetId() {
    return elid;
  }

  template <typename A>
  static bool fromJSArray(std::vector<A> & outArr, Local<Array> &arr) {
    for (unsigned int i = 0; i < arr->Length(); ++i) {
      A * v = NoCLUnwrap<A>(arr->Get(i));
      if (v == NULL) {
        return false;
      }
      outArr.push_back(*v);
    }

    return true;
  }

  template <typename U>
  static std::vector<T> toCLArray(std::vector<U> outArr) {
    std::vector<T> out;
    for (unsigned int i = 0; i < outArr.size(); ++i) {
      out.push_back(outArr[i].getRaw());
    }

    return out;
  }

  static cl_uint getErrorCode(){
    return err;
  }

};

#define NOCL_TO_ARRAY(TO, FROM, TYPE) \
  if (!TYPE::fromJSArray<TYPE>(TO, FROM)) { \
    NanThrowError(opencl::getExceptionMessage(TYPE::getErrorCode()).c_str(), TYPE::getErrorCode()); \
    NanReturnUndefined();\
  }

#define NOCL_TO_CL_ARRAY(FROM, TYPE) \
  FROM.size() ? &TYPE::toCLArray<TYPE>(FROM).front() : nullptr

class NoCLPlatformId : public NoCLObject<cl_platform_id, 0, CL_INVALID_PLATFORM> {

public:
  NoCLPlatformId(cl_platform_id raw) : NoCLObject(raw) {
  }
};

class NoCLDeviceId : public NoCLObject<cl_device_id, 1, CL_INVALID_DEVICE> {

public:
  NoCLDeviceId(cl_device_id raw) : NoCLObject(raw) {
  }
};

class NoCLContext : public NoCLObject<cl_context, 2, CL_INVALID_CONTEXT> {

public:
  NoCLContext(cl_context raw) : NoCLObject(raw) {
  }
};

class NoCLProgram : public NoCLObject<cl_program, 3, CL_INVALID_PROGRAM> {

public:
  NoCLProgram(cl_program raw) : NoCLObject(raw) {
  }
};

class NoCLKernel : public NoCLObject<cl_kernel, 4, CL_INVALID_KERNEL> {

public:
  NoCLKernel(cl_kernel raw) : NoCLObject(raw) {
  }
};

class NoCLMem : public NoCLObject<cl_mem, 5, CL_INVALID_MEM_OBJECT> {

public:
  NoCLMem(cl_mem raw) : NoCLObject(raw) {
  }
};

class NoCLSampler : public NoCLObject<cl_sampler, 6, CL_INVALID_SAMPLER> {

public:
  NoCLSampler(cl_sampler raw) : NoCLObject(raw) {
  }
};

class NoCLCommandQueue : public NoCLObject<cl_command_queue, 7, CL_INVALID_COMMAND_QUEUE> {

public:
  NoCLCommandQueue(cl_command_queue raw) : NoCLObject(raw) {
  }
};

class NoCLEvent : public NoCLObject<cl_event, 8, CL_INVALID_EVENT> {

public:
  NoCLEvent(cl_event raw) : NoCLObject(raw) {
  }
};

class NoCLProgramBinary : public NoCLObject<const unsigned char *, 9, CL_INVALID_VALUE> {

public:
  NoCLProgramBinary(const unsigned char * raw) : NoCLObject(raw) {
  }
};

class NoCLMappedPtr : public NoCLObject<void *, 10, CL_INVALID_VALUE> {

public:
  NoCLMappedPtr(void * raw) : NoCLObject(raw) {
  }
};

NAN_METHOD(Equals);

// FIXME static does not seem to work great with V8 (random segfaults)
// But we should not create a template each time we create an object
template <typename T>
Local<Object> NoCLWrapCLObject(T * elm) {
  Local<ObjectTemplate> tpl = NanNew<ObjectTemplate>();

  tpl->Set(NanNew<v8::String>("equals"),
    NanNew<FunctionTemplate>(Equals, NanNew<v8::External>(elm)));

  tpl->SetInternalFieldCount(2);
  Local<Object> obj = tpl->NewInstance();

  NanSetInternalFieldPointer(obj, 0, elm);
  NanSetInternalFieldPointer(obj, 1, new unsigned int(T::GetId()));
  return obj;
}

#define NOCL_WRAP(T, V) \
  NoCLWrapCLObject<T>(new T(V))

#endif

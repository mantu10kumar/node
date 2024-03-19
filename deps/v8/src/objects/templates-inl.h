// Copyright 2018 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_OBJECTS_TEMPLATES_INL_H_
#define V8_OBJECTS_TEMPLATES_INL_H_

#include "src/heap/heap-write-barrier-inl.h"
#include "src/objects/objects-inl.h"
#include "src/objects/oddball.h"
#include "src/objects/shared-function-info.h"
#include "src/objects/templates.h"

// Has to be the last include (doesn't have include guards):
#include "src/objects/object-macros.h"

namespace v8 {
namespace internal {

#include "torque-generated/src/objects/templates-tq-inl.inc"

TQ_OBJECT_CONSTRUCTORS_IMPL(TemplateInfo)
TQ_OBJECT_CONSTRUCTORS_IMPL(FunctionTemplateInfo)
TQ_OBJECT_CONSTRUCTORS_IMPL(ObjectTemplateInfo)
TQ_OBJECT_CONSTRUCTORS_IMPL(FunctionTemplateRareData)
TQ_OBJECT_CONSTRUCTORS_IMPL(DictionaryTemplateInfo)

NEVER_READ_ONLY_SPACE_IMPL(DictionaryTemplateInfo)
NEVER_READ_ONLY_SPACE_IMPL(ObjectTemplateInfo)

BOOL_ACCESSORS(FunctionTemplateInfo, relaxed_flag,
               is_object_template_call_handler,
               IsObjectTemplateCallHandlerBit::kShift)
BOOL_ACCESSORS(FunctionTemplateInfo, relaxed_flag, has_side_effects,
               HasSideEffectsBit::kShift)
BOOL_ACCESSORS(FunctionTemplateInfo, relaxed_flag, undetectable,
               UndetectableBit::kShift)
BOOL_ACCESSORS(FunctionTemplateInfo, relaxed_flag, needs_access_check,
               NeedsAccessCheckBit::kShift)
BOOL_ACCESSORS(FunctionTemplateInfo, relaxed_flag, read_only_prototype,
               ReadOnlyPrototypeBit::kShift)
BOOL_ACCESSORS(FunctionTemplateInfo, relaxed_flag, remove_prototype,
               RemovePrototypeBit::kShift)
BOOL_ACCESSORS(FunctionTemplateInfo, relaxed_flag, accept_any_receiver,
               AcceptAnyReceiverBit::kShift)
BOOL_ACCESSORS(FunctionTemplateInfo, relaxed_flag, published,
               PublishedBit::kShift)

BIT_FIELD_ACCESSORS(
    FunctionTemplateInfo, relaxed_flag,
    allowed_receiver_instance_type_range_start,
    FunctionTemplateInfo::AllowedReceiverInstanceTypeRangeStartBits)
BIT_FIELD_ACCESSORS(
    FunctionTemplateInfo, relaxed_flag,
    allowed_receiver_instance_type_range_end,
    FunctionTemplateInfo::AllowedReceiverInstanceTypeRangeEndBits)

RELAXED_UINT32_ACCESSORS(FunctionTemplateInfo, flag,
                         FunctionTemplateInfo::kFlagOffset)

int32_t FunctionTemplateInfo::relaxed_flag() const {
  return flag(kRelaxedLoad);
}
void FunctionTemplateInfo::set_relaxed_flag(int32_t flags) {
  return set_flag(flags, kRelaxedStore);
}

Address FunctionTemplateInfo::callback(i::IsolateForSandbox isolate) const {
  Address result = maybe_redirected_callback(isolate);
  if (!USE_SIMULATOR_BOOL) return result;
  if (result == kNullAddress) return kNullAddress;
  return ExternalReference::UnwrapRedirection(result);
}

void FunctionTemplateInfo::init_callback(i::IsolateForSandbox isolate,
                                         Address initial_value) {
  init_maybe_redirected_callback(isolate, initial_value);
  if (USE_SIMULATOR_BOOL) {
    init_callback_redirection(isolate);
  }
}

void FunctionTemplateInfo::set_callback(i::IsolateForSandbox isolate,
                                        Address value) {
  set_maybe_redirected_callback(isolate, value);
  if (USE_SIMULATOR_BOOL) {
    init_callback_redirection(isolate);
  }
}

void FunctionTemplateInfo::init_callback_redirection(
    i::IsolateForSandbox isolate) {
  CHECK(USE_SIMULATOR_BOOL);
  Address value = maybe_redirected_callback(isolate);
  if (value == kNullAddress) return;
  value =
      ExternalReference::Redirect(value, ExternalReference::DIRECT_API_CALL);
  set_maybe_redirected_callback(isolate, value);
}

void FunctionTemplateInfo::remove_callback_redirection(
    i::IsolateForSandbox isolate) {
  CHECK(USE_SIMULATOR_BOOL);
  Address value = callback(isolate);
  set_maybe_redirected_callback(isolate, value);
}

EXTERNAL_POINTER_ACCESSORS_MAYBE_READ_ONLY_HOST(
    FunctionTemplateInfo, maybe_redirected_callback, Address,
    kMaybeRedirectedCallbackOffset, kFunctionTemplateInfoCallbackTag)

template <class IsolateT>
bool FunctionTemplateInfo::has_callback(IsolateT* isolate) const {
  return !IsTheHole(callback_data(kAcquireLoad), isolate);
}

// static
Tagged<FunctionTemplateRareData>
FunctionTemplateInfo::EnsureFunctionTemplateRareData(
    Isolate* isolate, Handle<FunctionTemplateInfo> function_template_info) {
  Tagged<HeapObject> extra =
      function_template_info->rare_data(isolate, kAcquireLoad);
  if (IsUndefined(extra, isolate)) {
    return AllocateFunctionTemplateRareData(isolate, function_template_info);
  } else {
    return FunctionTemplateRareData::cast(extra);
  }
}

#define RARE_ACCESSORS(Name, CamelName, Type, Default)                         \
  DEF_GETTER(FunctionTemplateInfo, Get##CamelName, Tagged<Type>) {             \
    Tagged<HeapObject> extra = rare_data(cage_base, kAcquireLoad);             \
    Tagged<Undefined> undefined =                                              \
        GetReadOnlyRoots(cage_base).undefined_value();                         \
    return extra == undefined ? Default                                        \
                              : FunctionTemplateRareData::cast(extra)->Name(); \
  }                                                                            \
  inline void FunctionTemplateInfo::Set##CamelName(                            \
      Isolate* isolate, Handle<FunctionTemplateInfo> function_template_info,   \
      Handle<Type> Name) {                                                     \
    Tagged<FunctionTemplateRareData> rare_data =                               \
        EnsureFunctionTemplateRareData(isolate, function_template_info);       \
    rare_data->set_##Name(*Name);                                              \
  }

RARE_ACCESSORS(prototype_template, PrototypeTemplate, HeapObject, undefined)
RARE_ACCESSORS(prototype_provider_template, PrototypeProviderTemplate,
               HeapObject, undefined)
RARE_ACCESSORS(parent_template, ParentTemplate, HeapObject, undefined)
RARE_ACCESSORS(named_property_handler, NamedPropertyHandler, HeapObject,
               undefined)
RARE_ACCESSORS(indexed_property_handler, IndexedPropertyHandler, HeapObject,
               undefined)
RARE_ACCESSORS(instance_template, InstanceTemplate, HeapObject, undefined)
RARE_ACCESSORS(instance_call_handler, InstanceCallHandler, HeapObject,
               undefined)
RARE_ACCESSORS(access_check_info, AccessCheckInfo, HeapObject, undefined)
RARE_ACCESSORS(c_function_overloads, CFunctionOverloads, FixedArray,
               GetReadOnlyRoots(cage_base).empty_fixed_array())
#undef RARE_ACCESSORS

InstanceType FunctionTemplateInfo::GetInstanceType() const {
  int type = instance_type();
  DCHECK(base::IsInRange(type, Internals::kFirstJSApiObjectType,
                         Internals::kLastJSApiObjectType));
  return static_cast<InstanceType>(type);
}

void FunctionTemplateInfo::SetInstanceType(int api_instance_type) {
  // Translate |api_instance_type| value from range
  // [Internals::kFirstEmbedderJSApiObjectType,
  //  Internals::kLastEmbedderJSApiObjectType] to range
  // [Internals::kFirstJSApiObjectType, Internals::kLastJSApiObjectType].
  DCHECK_LE(Internals::kFirstEmbedderJSApiObjectType, api_instance_type);
  DCHECK_LE(api_instance_type, Internals::kLastEmbedderJSApiObjectType);
  // kNoJSApiObjectType must correspond to JS_API_OBJECT_TYPE.
  static_assert(kNoJSApiObjectType == 0);
  static_assert(JS_API_OBJECT_TYPE == Internals::kFirstJSApiObjectType);
  set_instance_type(static_cast<InstanceType>(
      api_instance_type + Internals::kFirstJSApiObjectType));
}

void FunctionTemplateInfo::SetAllowedReceiverInstanceTypeRange(
    int api_instance_type_start, int api_instance_type_end) {
  // Translate |api_instance_type_start| and |api_instance_type_end| values
  // from range [Internals::kFirstEmbedderJSApiObjectType,
  //             Internals::kLastEmbedderJSApiObjectType] to range
  // [Internals::kFirstJSApiObjectType, Internals::kLastJSApiObjectType].
  DCHECK_LE(Internals::kFirstEmbedderJSApiObjectType, api_instance_type_start);
  DCHECK_LE(api_instance_type_start, api_instance_type_end);
  DCHECK_LE(api_instance_type_end, Internals::kLastEmbedderJSApiObjectType);
  // kNoJSApiObjectType must correspond to JS_API_OBJECT_TYPE.
  static_assert(kNoJSApiObjectType == 0);
  static_assert(JS_API_OBJECT_TYPE == Internals::kFirstJSApiObjectType);
  set_allowed_receiver_instance_type_range_start(static_cast<InstanceType>(
      api_instance_type_start + Internals::kFirstJSApiObjectType));
  set_allowed_receiver_instance_type_range_end(static_cast<InstanceType>(
      api_instance_type_end + Internals::kFirstJSApiObjectType));
}

// Ensure that instance type fields in FunctionTemplateInfo are big enough
// to fit the whole JSApiObject type range.
static_assert(
    FunctionTemplateInfo::AllowedReceiverInstanceTypeRangeStartBits::is_valid(
        LAST_JS_API_OBJECT_TYPE));
static_assert(
    FunctionTemplateInfo::AllowedReceiverInstanceTypeRangeEndBits::is_valid(
        LAST_JS_API_OBJECT_TYPE));

bool TemplateInfo::should_cache() const {
  return serial_number() != kDoNotCache;
}
bool TemplateInfo::is_cached() const { return serial_number() > kUncached; }

bool FunctionTemplateInfo::instantiated() {
  return IsSharedFunctionInfo(shared_function_info());
}

inline bool FunctionTemplateInfo::BreakAtEntry(Isolate* isolate) {
  Tagged<Object> maybe_shared = shared_function_info();
  if (IsSharedFunctionInfo(maybe_shared)) {
    Tagged<SharedFunctionInfo> shared = SharedFunctionInfo::cast(maybe_shared);
    return shared->BreakAtEntry(isolate);
  }
  return false;
}

Tagged<FunctionTemplateInfo> FunctionTemplateInfo::GetParent(Isolate* isolate) {
  Tagged<Object> parent = GetParentTemplate();
  return IsUndefined(parent, isolate) ? Tagged<FunctionTemplateInfo>{}
                                      : FunctionTemplateInfo::cast(parent);
}

Tagged<ObjectTemplateInfo> ObjectTemplateInfo::GetParent(Isolate* isolate) {
  Tagged<Object> maybe_ctor = constructor();
  if (IsUndefined(maybe_ctor, isolate)) return ObjectTemplateInfo();
  Tagged<FunctionTemplateInfo> constructor =
      FunctionTemplateInfo::cast(maybe_ctor);
  while (true) {
    constructor = constructor->GetParent(isolate);
    if (constructor.is_null()) return ObjectTemplateInfo();
    Tagged<Object> maybe_obj = constructor->GetInstanceTemplate();
    if (!IsUndefined(maybe_obj, isolate)) {
      return ObjectTemplateInfo::cast(maybe_obj);
    }
  }
  return Tagged<ObjectTemplateInfo>();
}

int ObjectTemplateInfo::embedder_field_count() const {
  return EmbedderFieldCountBits::decode(data());
}

void ObjectTemplateInfo::set_embedder_field_count(int count) {
  DCHECK_LE(count, JSObject::kMaxEmbedderFields);
  return set_data(EmbedderFieldCountBits::update(data(), count));
}

bool ObjectTemplateInfo::immutable_proto() const {
  return IsImmutablePrototypeBit::decode(data());
}

void ObjectTemplateInfo::set_immutable_proto(bool immutable) {
  return set_data(IsImmutablePrototypeBit::update(data(), immutable));
}

bool ObjectTemplateInfo::code_like() const {
  return IsCodeKindBit::decode(data());
}

void ObjectTemplateInfo::set_code_like(bool is_code_like) {
  return set_data(IsCodeKindBit::update(data(), is_code_like));
}

bool FunctionTemplateInfo::IsTemplateFor(Tagged<JSObject> object) const {
  return IsTemplateFor(object->map());
}

bool TemplateInfo::TryGetIsolate(Isolate** isolate) const {
  if (GetIsolateFromHeapObject(*this, isolate)) return true;
  Isolate* isolate_value = Isolate::TryGetCurrent();
  if (isolate_value != nullptr) {
    *isolate = isolate_value;
    return true;
  }
  return false;
}

Isolate* TemplateInfo::GetIsolateChecked() const {
  Isolate* isolate;
  CHECK(TryGetIsolate(&isolate));
  return isolate;
}

}  // namespace internal
}  // namespace v8

#include "src/objects/object-macros-undef.h"

#endif  // V8_OBJECTS_TEMPLATES_INL_H_

#ifndef C_MEDIAPIPE_API_FRAMEWORK_PACKET_H_
#define C_MEDIAPIPE_API_FRAMEWORK_PACKET_H_

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "mediapipe/framework/packet.h"
#include "mediapipe_api/common.h"
#include "mediapipe_api/external/protobuf.h"

template <typename T>
class UnsafePacketHolder : public mediapipe::packet_internal::Holder<T> {
  using mediapipe::packet_internal::Holder<T>::ptr_;

 public:
  const T* Get() const {
    return ptr_;
  }
};

extern "C" {

typedef std::map<std::string, mediapipe::Packet> SidePacket;

/** mediapipe::Packet API */
MP_CAPI(MpReturnCode) mp_Packet__(mediapipe::Packet** packet_out);
MP_CAPI(void) mp_Packet__delete(mediapipe::Packet* packet);
MP_CAPI(MpReturnCode) mp_Packet__At__Rtimestamp(mediapipe::Packet* packet, mediapipe::Timestamp* timestamp, mediapipe::Packet** packet_out);
MP_CAPI(MpReturnCode) mp_Packet__ValidateAsProtoMessageLite(mediapipe::Packet* packet, mediapipe::Status** status_out);
MP_CAPI(MpReturnCode) mp_Packet__Timestamp(mediapipe::Packet* packet, mediapipe::Timestamp** timestamp_out);
MP_CAPI(MpReturnCode) mp_Packet__DebugString(mediapipe::Packet* packet, const char** str_out);
MP_CAPI(MpReturnCode) mp_Packet__RegisteredTypeName(mediapipe::Packet* packet, const char** str_out);
MP_CAPI(MpReturnCode) mp_Packet__DebugTypeName(mediapipe::Packet* packet, const char** str_out);

// Boolean
MP_CAPI(MpReturnCode) mp__MakeBoolPacket__b(bool value, mediapipe::Packet** packet_out);
MP_CAPI(MpReturnCode) mp__MakeBoolPacket_At__b_Rtimestamp(bool value, mediapipe::Timestamp* timestamp, mediapipe::Packet** packet_out);
MP_CAPI(MpReturnCode) mp_Packet__GetBool(mediapipe::Packet* packet, bool* value_out);
MP_CAPI(MpReturnCode) mp_Packet__ValidateAsBool(mediapipe::Packet* packet, mediapipe::Status** status_out);

// Float
MP_CAPI(MpReturnCode) mp__MakeFloatPacket__f(float value, mediapipe::Packet** packet_out);
MP_CAPI(MpReturnCode) mp__MakeFloatPacket_At__f_Rtimestamp(float value, mediapipe::Timestamp* timestamp, mediapipe::Packet** packet_out);
MP_CAPI(MpReturnCode) mp_Packet__GetFloat(mediapipe::Packet* packet, float* value_out);
MP_CAPI(MpReturnCode) mp_Packet__ValidateAsFloat(mediapipe::Packet* packet, mediapipe::Status** status_out);

// Int
MP_CAPI(MpReturnCode) mp__MakeIntPacket__i(int value, mediapipe::Packet** packet_out);
MP_CAPI(MpReturnCode) mp__MakeIntPacket_At__i_Rtimestamp(int value, mediapipe::Timestamp* timestamp, mediapipe::Packet** packet_out);
MP_CAPI(MpReturnCode) mp_Packet__GetInt(mediapipe::Packet* packet, int* value_out);
MP_CAPI(MpReturnCode) mp_Packet__ValidateAsInt(mediapipe::Packet* packet, mediapipe::Status** status_out);

// String
MP_CAPI(MpReturnCode) mp__MakeStringPacket__PKc(const char* str, mediapipe::Packet** packet_out);
MP_CAPI(MpReturnCode) mp__MakeStringPacket_At__PKc_Rtimestamp(const char* str,
                                                              mediapipe::Timestamp* timestamp,
                                                              mediapipe::Packet** packet_out);
MP_CAPI(MpReturnCode) mp_Packet__GetString(mediapipe::Packet* packet, const char** value_out);
MP_CAPI(MpReturnCode) mp_Packet__ValidateAsString(mediapipe::Packet* packet, mediapipe::Status** status_out);

/** SidePacket API */
MP_CAPI(MpReturnCode) mp_SidePacket__(SidePacket** side_packet_out);
MP_CAPI(void) mp_SidePacket__delete(SidePacket* side_packet);
MP_CAPI(MpReturnCode) mp_SidePacket__emplace__PKc_Rpacket(SidePacket* side_packet, const char* key, mediapipe::Packet* packet);
MP_CAPI(MpReturnCode) mp_SidePacket__at__PKc(SidePacket* side_packet, const char* key, mediapipe::Packet** packet_out);
MP_CAPI(MpReturnCode) mp_SidePacket__erase__PKc(SidePacket* side_packet, const char* key, int* count_out);
MP_CAPI(void) mp_SidePacket__clear(SidePacket* side_packet);
MP_CAPI(int) mp_SidePacket__size(SidePacket* side_packet);

}  // extern "C"

template <class T>
inline MpReturnCode mp_Packet__Consume(mediapipe::Packet* packet, mediapipe::StatusOr<T>** status_or_value_out) {
  TRY_ALL {
    auto status_or_value = packet->Consume<T>();

    if (status_or_value.ok()) {
      *status_or_value_out = new mediapipe::StatusOr<T> { std::move(*status_or_value.ConsumeValueOrDie().release()) };
    } else {
      *status_or_value_out = new mediapipe::StatusOr<T> { status_or_value.status() };
    }
    RETURN_CODE(MpReturnCode::Success);
  } CATCH_ALL
}

template <class T>
inline MpReturnCode mp_Packet__Get(mediapipe::Packet* packet, const T** value_out) {
  TRY_ALL {
    auto holder = packet->IsEmpty() ? nullptr : mediapipe::packet_internal::GetHolder(*packet)->As<T>();
    auto unsafe_holder = static_cast<const UnsafePacketHolder<T>*>(holder);

    if (unsafe_holder == nullptr) {
      mediapipe::Status status = packet->ValidateAsType<T>();
      LOG(FATAL) << "mp_Packet__Get() failed: " << status.message();
    }
    *value_out = unsafe_holder->Get();
    RETURN_CODE(MpReturnCode::Success);
  } CATCH_ALL
}

template <class T>
inline MpReturnCode mp_Packet__GetSerializedProto(mediapipe::Packet* packet, mp_api::SerializedProto** value_out) {
  TRY_ALL {
    auto proto = packet->Get<T>();
    *value_out = SerializeProto(proto);
    RETURN_CODE(MpReturnCode::Success);
  } CATCH_ALL
}

template <class T>
inline MpReturnCode mp_Packet__GetSerializedProtoVector(mediapipe::Packet* packet, mp_api::SerializedProtoVector** value_out) {
  TRY_ALL {
    auto proto_vec = packet->Get<std::vector<T>>();
    *value_out = SerializeProtoVector(proto_vec);
    RETURN_CODE(MpReturnCode::Success);
  } CATCH_ALL
}

#endif  // C_MEDIAPIPE_API_FRAMEWORK_PACKET_H_

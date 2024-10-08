// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_PACKETS_MY_PROTOCOL_H_
#define FLATBUFFERS_GENERATED_PACKETS_MY_PROTOCOL_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace my {
namespace protocol {

struct AuthRequest;
struct AuthRequestBuilder;

struct AuthResponse;
struct AuthResponseBuilder;

struct ThisIsNotAPacket;
struct ThisIsNotAPacketBuilder;

enum AccountStatus : uint8_t {
  AccountStatus_Active = 0,
  AccountStatus_Unconfirmed = 1,
  AccountStatus_Blocked = 2,
  AccountStatus_MIN = AccountStatus_Active,
  AccountStatus_MAX = AccountStatus_Blocked
};

inline const AccountStatus (&EnumValuesAccountStatus())[3] {
  static const AccountStatus values[] = {
    AccountStatus_Active,
    AccountStatus_Unconfirmed,
    AccountStatus_Blocked
  };
  return values;
}

inline const char * const *EnumNamesAccountStatus() {
  static const char * const names[4] = {
    "Active",
    "Unconfirmed",
    "Blocked",
    nullptr
  };
  return names;
}

inline const char *EnumNameAccountStatus(AccountStatus e) {
  if (::flatbuffers::IsOutRange(e, AccountStatus_Active, AccountStatus_Blocked)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesAccountStatus()[index];
}

struct AuthRequest FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef AuthRequestBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_EMAIL = 4,
    VT_PASSWORD = 6
  };
  const ::flatbuffers::String *email() const {
    return GetPointer<const ::flatbuffers::String *>(VT_EMAIL);
  }
  const ::flatbuffers::String *password() const {
    return GetPointer<const ::flatbuffers::String *>(VT_PASSWORD);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_EMAIL) &&
           verifier.VerifyString(email()) &&
           VerifyOffset(verifier, VT_PASSWORD) &&
           verifier.VerifyString(password()) &&
           verifier.EndTable();
  }
};

struct AuthRequestBuilder {
  typedef AuthRequest Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_email(::flatbuffers::Offset<::flatbuffers::String> email) {
    fbb_.AddOffset(AuthRequest::VT_EMAIL, email);
  }
  void add_password(::flatbuffers::Offset<::flatbuffers::String> password) {
    fbb_.AddOffset(AuthRequest::VT_PASSWORD, password);
  }
  explicit AuthRequestBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<AuthRequest> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<AuthRequest>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<AuthRequest> CreateAuthRequest(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> email = 0,
    ::flatbuffers::Offset<::flatbuffers::String> password = 0) {
  AuthRequestBuilder builder_(_fbb);
  builder_.add_password(password);
  builder_.add_email(email);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<AuthRequest> CreateAuthRequestDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *email = nullptr,
    const char *password = nullptr) {
  auto email__ = email ? _fbb.CreateString(email) : 0;
  auto password__ = password ? _fbb.CreateString(password) : 0;
  return my::protocol::CreateAuthRequest(
      _fbb,
      email__,
      password__);
}

struct AuthResponse FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef AuthResponseBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SUCCESS = 4,
    VT_STATUS = 6
  };
  bool success() const {
    return GetField<uint8_t>(VT_SUCCESS, 0) != 0;
  }
  my::protocol::AccountStatus status() const {
    return static_cast<my::protocol::AccountStatus>(GetField<uint8_t>(VT_STATUS, 0));
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_SUCCESS, 1) &&
           VerifyField<uint8_t>(verifier, VT_STATUS, 1) &&
           verifier.EndTable();
  }
};

struct AuthResponseBuilder {
  typedef AuthResponse Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_success(bool success) {
    fbb_.AddElement<uint8_t>(AuthResponse::VT_SUCCESS, static_cast<uint8_t>(success), 0);
  }
  void add_status(my::protocol::AccountStatus status) {
    fbb_.AddElement<uint8_t>(AuthResponse::VT_STATUS, static_cast<uint8_t>(status), 0);
  }
  explicit AuthResponseBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<AuthResponse> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<AuthResponse>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<AuthResponse> CreateAuthResponse(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    bool success = false,
    my::protocol::AccountStatus status = my::protocol::AccountStatus_Active) {
  AuthResponseBuilder builder_(_fbb);
  builder_.add_status(status);
  builder_.add_success(success);
  return builder_.Finish();
}

struct ThisIsNotAPacket FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ThisIsNotAPacketBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_TEST = 4
  };
  const ::flatbuffers::String *test() const {
    return GetPointer<const ::flatbuffers::String *>(VT_TEST);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_TEST) &&
           verifier.VerifyString(test()) &&
           verifier.EndTable();
  }
};

struct ThisIsNotAPacketBuilder {
  typedef ThisIsNotAPacket Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_test(::flatbuffers::Offset<::flatbuffers::String> test) {
    fbb_.AddOffset(ThisIsNotAPacket::VT_TEST, test);
  }
  explicit ThisIsNotAPacketBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ThisIsNotAPacket> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ThisIsNotAPacket>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<ThisIsNotAPacket> CreateThisIsNotAPacket(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> test = 0) {
  ThisIsNotAPacketBuilder builder_(_fbb);
  builder_.add_test(test);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<ThisIsNotAPacket> CreateThisIsNotAPacketDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *test = nullptr) {
  auto test__ = test ? _fbb.CreateString(test) : 0;
  return my::protocol::CreateThisIsNotAPacket(
      _fbb,
      test__);
}

}  // namespace protocol
}  // namespace my

#endif  // FLATBUFFERS_GENERATED_PACKETS_MY_PROTOCOL_H_

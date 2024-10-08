#include <cryptopp/sha.h>
#include <cryptopp/hex.h>

#include "./hash.hpp"

using namespace std;
using namespace CryptoPP;

string hashes::sha1(const string& data) {
  using byte = CryptoPP::byte;

  byte digest[SHA1::DIGESTSIZE];

  SHA1().CalculateDigest(digest, (const uint8_t*)data.data(), data.size());

  string encoded;

  HexEncoder encoder;
  encoder.Attach(new StringSink(encoded));
  encoder.Put(digest, sizeof(digest));
  encoder.MessageEnd();

  return encoded;
}

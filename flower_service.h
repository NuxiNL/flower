#ifndef APROTOC_425c3c08bd7fcf5ebbbf26fdc1f15c5af1e256aa8f22a130b24bcac287f045dc
#define APROTOC_425c3c08bd7fcf5ebbbf26fdc1f15c5af1e256aa8f22a130b24bcac287f045dc

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <argdata.h>
#include <arpc++/arpc++.h>

namespace flower_service {
namespace {

class AcceptOptions final : public arpc::Message {
 public:
  void Parse(const argdata_t& ad) override {
  }

 private:
};

class ClientChannel final : public arpc::Message {
 public:
  ClientChannel() : channel_(0) {}

  void Parse(const argdata_t& ad) override {
    argdata_map_iterator_t it;
    argdata_map_iterate(&ad, &it);
    const argdata_t* key;
    const argdata_t* value;
    while (argdata_map_next(&it, &key, &value)) {
      const char* keystr;
      std::size_t keylen;
      if (argdata_get_str(key, &keystr, &keylen) != 0)
        continue;
      std::string_view keyss(keystr, keylen);
      if (keyss == "channel") {
        argdata_get_int(value, &channel_);
      }
    }
  }

  std::int32_t channel() const { return channel_; }
  void set_channel(std::int32_t value) { channel_ = value; }
  void clear_channel() { channel_ = 0; }

 private:
  std::int32_t channel_;
};

class ClientChannelOptions final : public arpc::Message {
 public:
  void Parse(const argdata_t& ad) override {
    argdata_map_iterator_t it;
    argdata_map_iterate(&ad, &it);
    const argdata_t* key;
    const argdata_t* value;
    while (argdata_map_next(&it, &key, &value)) {
      const char* keystr;
      std::size_t keylen;
      if (argdata_get_str(key, &keystr, &keylen) != 0)
        continue;
      std::string_view keyss(keystr, keylen);
      if (keyss == "additional_constraints") {
        argdata_map_iterator_t it2;
        argdata_map_iterate(value, &it2);
        const argdata_t* key2, *value2;
        while (argdata_map_next(&it2, &key2, &value2)) {
          const char* value2str;
          std::size_t value2len;
          if (argdata_get_str(value2, &value2str, &value2len) != 0)
            continue;
          std::string_view mapkey(value2str, value2len);
          const char* key2str;
          std::size_t key2len;
          if (argdata_get_str(key2, &key2str, &key2len) == 0)
            additional_constraints_.emplace(mapkey, std::string()).first->second = std::string_view(key2str, key2len);
        }
      }
    }
  }

  const std::map<std::string, std::string, std::less<>>& additional_constraints() const { return additional_constraints_; }
  std::map<std::string, std::string, std::less<>>* mutable_additional_constraints() { return &additional_constraints_; }

 private:
  std::map<std::string, std::string, std::less<>> additional_constraints_;
};

class ConnectOptions final : public arpc::Message {
 public:
  void Parse(const argdata_t& ad) override {
  }

 private:
};

class Connection final : public arpc::Message {
 public:
  Connection() : socket_(0) {}

  void Parse(const argdata_t& ad) override {
    argdata_map_iterator_t it;
    argdata_map_iterate(&ad, &it);
    const argdata_t* key;
    const argdata_t* value;
    while (argdata_map_next(&it, &key, &value)) {
      const char* keystr;
      std::size_t keylen;
      if (argdata_get_str(key, &keystr, &keylen) != 0)
        continue;
      std::string_view keyss(keystr, keylen);
      if (keyss == "constraints") {
        argdata_map_iterator_t it2;
        argdata_map_iterate(value, &it2);
        const argdata_t* key2, *value2;
        while (argdata_map_next(&it2, &key2, &value2)) {
          const char* value2str;
          std::size_t value2len;
          if (argdata_get_str(value2, &value2str, &value2len) != 0)
            continue;
          std::string_view mapkey(value2str, value2len);
          const char* key2str;
          std::size_t key2len;
          if (argdata_get_str(key2, &key2str, &key2len) == 0)
            constraints_.emplace(mapkey, std::string()).first->second = std::string_view(key2str, key2len);
        }
      } else if (keyss == "socket") {
        argdata_get_int(value, &socket_);
      }
    }
  }

  const std::map<std::string, std::string, std::less<>>& constraints() const { return constraints_; }
  std::map<std::string, std::string, std::less<>>* mutable_constraints() { return &constraints_; }

  std::int32_t socket() const { return socket_; }
  void set_socket(std::int32_t value) { socket_ = value; }
  void clear_socket() { socket_ = 0; }

 private:
  std::map<std::string, std::string, std::less<>> constraints_;
  std::int32_t socket_;
};

class ServerChannel final : public arpc::Message {
 public:
  ServerChannel() : channel_(0) {}

  void Parse(const argdata_t& ad) override {
    argdata_map_iterator_t it;
    argdata_map_iterate(&ad, &it);
    const argdata_t* key;
    const argdata_t* value;
    while (argdata_map_next(&it, &key, &value)) {
      const char* keystr;
      std::size_t keylen;
      if (argdata_get_str(key, &keystr, &keylen) != 0)
        continue;
      std::string_view keyss(keystr, keylen);
      if (keyss == "channel") {
        argdata_get_int(value, &channel_);
      }
    }
  }

  std::int32_t channel() const { return channel_; }
  void set_channel(std::int32_t value) { channel_ = value; }
  void clear_channel() { channel_ = 0; }

 private:
  std::int32_t channel_;
};

class ServerChannelOptions final : public arpc::Message {
 public:
  void Parse(const argdata_t& ad) override {
    argdata_map_iterator_t it;
    argdata_map_iterate(&ad, &it);
    const argdata_t* key;
    const argdata_t* value;
    while (argdata_map_next(&it, &key, &value)) {
      const char* keystr;
      std::size_t keylen;
      if (argdata_get_str(key, &keystr, &keylen) != 0)
        continue;
      std::string_view keyss(keystr, keylen);
      if (keyss == "additional_constraints") {
        argdata_map_iterator_t it2;
        argdata_map_iterate(value, &it2);
        const argdata_t* key2, *value2;
        while (argdata_map_next(&it2, &key2, &value2)) {
          const char* value2str;
          std::size_t value2len;
          if (argdata_get_str(value2, &value2str, &value2len) != 0)
            continue;
          std::string_view mapkey(value2str, value2len);
          const char* key2str;
          std::size_t key2len;
          if (argdata_get_str(key2, &key2str, &key2len) == 0)
            additional_constraints_.emplace(mapkey, std::string()).first->second = std::string_view(key2str, key2len);
        }
      }
    }
  }

  const std::map<std::string, std::string, std::less<>>& additional_constraints() const { return additional_constraints_; }
  std::map<std::string, std::string, std::less<>>* mutable_additional_constraints() { return &additional_constraints_; }

 private:
  std::map<std::string, std::string, std::less<>> additional_constraints_;
};

namespace ClientChannelService {

class Service : public arpc::Service {
 public:
  virtual arpc::Status Connect(arpc::ServerContext* context, const ConnectOptions* request, Connection* response) {
    return arpc::Status(arpc::StatusCode::UNIMPLEMENTED, "Operation not provided by this implementation");
  }

  virtual arpc::Status CreateNewChannel(arpc::ServerContext* context, const ClientChannelOptions* request, ClientChannel* response) {
    return arpc::Status(arpc::StatusCode::UNIMPLEMENTED, "Operation not provided by this implementation");
  }

};

class Stub {
 public:
  explicit Stub(const std::shared_ptr<arpc::Channel>& channel)
      : channel_(channel) {}

  arpc::Status Connect(arpc::ClientContext* context, const ConnectOptions& request, Connection* response) {
    return arpc::Status(arpc::StatusCode::UNIMPLEMENTED, "TODO(ed)");
  }

  arpc::Status CreateNewChannel(arpc::ClientContext* context, const ClientChannelOptions& request, ClientChannel* response) {
    return arpc::Status(arpc::StatusCode::UNIMPLEMENTED, "TODO(ed)");
  }

 private:
  const std::shared_ptr<arpc::Channel> channel_;
};

std::unique_ptr<Stub> NewStub(const std::shared_ptr<arpc::Channel>& channel) {
  return std::make_unique<Stub>(channel);
}

}

namespace ServerChannelService {

class Service : public arpc::Service {
 public:
  virtual arpc::Status AcceptIncomingConnections(arpc::ServerContext* context, const AcceptOptions* request, arpc::ServerWriter<Connection>* writer) {
    return arpc::Status(arpc::StatusCode::UNIMPLEMENTED, "Operation not provided by this implementation");
  }

  virtual arpc::Status CreateNewChannel(arpc::ServerContext* context, const ServerChannelOptions* request, ServerChannel* response) {
    return arpc::Status(arpc::StatusCode::UNIMPLEMENTED, "Operation not provided by this implementation");
  }

};

class Stub {
 public:
  explicit Stub(const std::shared_ptr<arpc::Channel>& channel)
      : channel_(channel) {}

  std::unique_ptr<arpc::ClientReader<Connection>> AcceptIncomingConnections(arpc::ClientContext* context, const AcceptOptions& request) {
    return std::make_unique<arpc::ClientReader<Connection>>(channel_.get(), "ServerChannelService", "AcceptIncomingConnections", context, request);
  }

  arpc::Status CreateNewChannel(arpc::ClientContext* context, const ServerChannelOptions& request, ServerChannel* response) {
    return arpc::Status(arpc::StatusCode::UNIMPLEMENTED, "TODO(ed)");
  }

 private:
  const std::shared_ptr<arpc::Channel> channel_;
};

std::unique_ptr<Stub> NewStub(const std::shared_ptr<arpc::Channel>& channel) {
  return std::make_unique<Stub>(channel);
}

}

}
}

#endif

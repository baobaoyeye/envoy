#pragma once

#include <memory>

#include "test/common/upstream/health_check_fuzz.pb.validate.h"
#include "test/common/upstream/health_checker_impl_test_utils.h"
#include "test/fuzz/common.pb.h"

namespace Envoy {
namespace Upstream {

class HealthCheckFuzz {
public:
  HealthCheckFuzz() = default;
  // This will delegate to the specific classes
  void initializeAndReplay(test::common::upstream::HealthCheckTestCase input);
  enum class Type {
    HTTP,
    TCP,
    GRPC,
  };

  // The specific implementations of respond look into the respond proto, which has all three types
  // of response
  virtual void respond(test::common::upstream::Respond respond, bool last_action) PURE;

  virtual void initialize(test::common::upstream::HealthCheckTestCase input) PURE;
  virtual void triggerIntervalTimer(bool expect_client_create) PURE;
  virtual void triggerTimeoutTimer(bool last_action) PURE;
  virtual void raiseEvent(const Network::ConnectionEvent& event_type, bool last_action) PURE;

  virtual ~HealthCheckFuzz() = default;

private:
  Network::ConnectionEvent getEventTypeFromProto(const test::common::upstream::RaiseEvent& event);

  void replay(const test::common::upstream::HealthCheckTestCase& input);
};

class HttpHealthCheckFuzz : public HealthCheckFuzz, HttpHealthCheckerImplTestBase {
public:
  void allocHttpHealthCheckerFromProto(const envoy::config::core::v3::HealthCheck& config);
  void initialize(test::common::upstream::HealthCheckTestCase input) override;
  void respond(test::common::upstream::Respond respond, bool last_action) override;
  void triggerIntervalTimer(bool expect_client_create) override;
  void triggerTimeoutTimer(bool last_action) override;
  void raiseEvent(const Network::ConnectionEvent& event_type, bool last_action) override;
  ~HttpHealthCheckFuzz() override = default;

  // Determines whether the client gets reused or not after response
  bool reuse_connection_ = true;
};

class TcpHealthCheckFuzz : public HealthCheckFuzz, TcpHealthCheckerImplTestBase {
public:
  void allocTcpHealthCheckerFromProto(const envoy::config::core::v3::HealthCheck& config);
  void initialize(test::common::upstream::HealthCheckTestCase input) override;
  void respond(test::common::upstream::Respond respond, bool last_action) override;
  void triggerIntervalTimer(bool expect_client_create) override;
  void triggerTimeoutTimer(bool last_action) override;
  void raiseEvent(const Network::ConnectionEvent& event_type, bool last_action) override;
  ~TcpHealthCheckFuzz() override = default;

  // Determines whether the client gets reused or not after response
  bool reuse_connection_ = true;

  // Empty response induces a specific codepath in raiseEvent in case of connected, ignores the
  // binary field and only uses text.
  bool empty_response_ = true;
};

class GrpcHealthCheckFuzz : public HealthCheckFuzz, GrpcHealthCheckerImplTestBaseUtils {
public:
  void allocGrpcHealthCheckerFromProto(const envoy::config::core::v3::HealthCheck& config);
  void initialize(test::common::upstream::HealthCheckTestCase input) override;
  // This has three components, headers, raw bytes, and trailers
  void respond(test::common::upstream::Respond respond, bool last_action) override;
  void triggerIntervalTimer(bool expect_client_create) override;
  void triggerTimeoutTimer(bool last_action) override;
  void raiseEvent(const Network::ConnectionEvent& event_type, bool last_action) override;
  void raiseGoAway(bool no_error);
  ~GrpcHealthCheckFuzz() override = default;

  // Determines whether the client gets reused or not after response
  bool reuse_connection_ = true;

  // Determines whether a client closes after responds and timeouts. Exactly maps to
  // received_no_error_goaway_ in source code.
  bool received_no_error_goaway_ = false;
};

} // namespace Upstream
} // namespace Envoy

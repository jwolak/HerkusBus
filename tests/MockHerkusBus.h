#pragma once

#include <gmock/gmock.h>

#include "IHerkusBus.h"

namespace Herkus {

class MockHerkusBus : public IHerkusBus {
 public:
  MOCK_METHOD(void, Publish, (const std::string& topic, const json& message_payload), (override));
  MOCK_METHOD(void, Subscribe, (const std::string& topic, subscriber_callback sub_callback), (override));
};

}  // namespace Herkus

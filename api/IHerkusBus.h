/*
 * IHerkusBus.h
 *
 *  Created on: 2026
 *      Author: GitHub Copilot
 */

#pragma once

#include <functional>
#include <string>

#include "nlohmann/json.hpp"

namespace Herkus {
using json = nlohmann::json;
using subscriber_callback = std::function<void(const std::string& topic, const json& msg)>;

class IHerkusBus {
 public:
  virtual ~IHerkusBus() = default;

  virtual void Publish(const std::string& topic, const json& message_payload) = 0;
  virtual void Subscribe(const std::string& topic, subscriber_callback sub_callback) = 0;
};

}  // namespace Herkus

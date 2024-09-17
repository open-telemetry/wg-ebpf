/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <google/protobuf/util/json_util.h>

template <typename T> std::string get_request_json(const T &request)
{
  std::string request_json_str;
  google::protobuf::util::JsonPrintOptions json_print_options;
  json_print_options.add_whitespace = true;
  json_print_options.always_print_primitive_fields = true;
  google::protobuf::util::Status status = google::protobuf::util::MessageToJsonString(request, &request_json_str, json_print_options);
  if (!status.ok()) {
    std::cerr << "Failed to convert message to JSON: " << status.ToString() << std::endl;
    return "";
  }
  return request_json_str;
}

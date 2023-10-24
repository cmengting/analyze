/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra/proto_util.h"

#include <glog/logging.h>

#include <fstream>

#include "absl/status/status.h"

namespace misra {
namespace proto_util {

std::vector<std::string> splitLocation(std::string loc) {
  std::vector<std::string> splited_res;
  size_t start = 0, end = loc.find(":");
  while (end != std::string::npos) {
    splited_res.push_back(loc.substr(start, end - start));
    start = end + 1;
    end = loc.find(":", start);
  }
  splited_res.push_back(loc.substr(start));
  return splited_res;
}

void AddLocToResult(analyzer::proto::Result* result, std::string& loc) {
  std::vector<std::string> splited_res = splitLocation(loc);
  if (splited_res.size() < 2) {
    LOG(ERROR) << "Loc parse failed";
    return;
  }
  std::string loc_path = splited_res[0];
  int loc_line_number = stoi(splited_res[1]);
  analyzer::proto::ErrorLocation* location = result->add_locations();
  location->set_path(loc_path);
  location->set_line_number(loc_line_number);
}

analyzer::proto::Result* AddBasicResultToResultsList(
    analyzer::proto::ResultsList* results_list, const std::string& path,
    int line_number, const std::string& error_message, bool false_positive) {
  analyzer::proto::Result* result = results_list->add_results();
  result->set_path(path);
  result->set_line_number(line_number);
  result->set_error_message(error_message);
  result->set_false_positive(false_positive);
  return result;
}

analyzer::proto::Result* AddResultToResultsList(
    analyzer::proto::ResultsList* results_list, const std::string& path,
    int line_number, const std::string& error_message, bool false_positive) {
  analyzer::proto::Result* result = AddBasicResultToResultsList(
      results_list, path, line_number, error_message, false_positive);
  analyzer::proto::ErrorLocation* location = result->add_locations();
  location->set_path(path);
  location->set_line_number(line_number);
  return result;
}

analyzer::proto::Result* AddMultipleLocationsResultToResultsList(
    analyzer::proto::ResultsList* results_list, const std::string& path,
    int line_number, const std::string& error_message,
    std::vector<std::string> locations, bool false_positive /*= false*/) {
  analyzer::proto::Result* result = AddBasicResultToResultsList(
      results_list, path, line_number, error_message, false_positive);
  for (auto loc : locations) {
    AddLocToResult(result, loc);
  }
  return result;
}

absl::Status GenerateProtoFile(const analyzer::proto::ResultsList& results_list,
                               const std::string& path) {
  std::fstream output(path, std::ios::out | std::ios::trunc | std::ios::binary);

  if (!results_list.SerializeToOstream(&output)) {
    LOG(ERROR) << "Failed to write ResultsList";
    return absl::InvalidArgumentError("Failed to write ResultsList");
  }
  return absl::OkStatus();
}

absl::Status ParseFromProtoFile(const std::string& path,
                                analyzer::proto::ResultsList* results_list) {
  std::fstream input(path, std::ios::in | std::ios::binary);

  if (!input) {
    LOG(ERROR) << path << " not found";
    return absl::NotFoundError(absl::StrCat(path, " not found"));
  }
  if (!results_list->ParseFromIstream(&input)) {
    LOG(ERROR) << "Failed to parse ResultsList";
    return absl::InvalidArgumentError("Failed to parse ResultsList");
  }
  return absl::OkStatus();
}

absl::Status ParseFromProtoString(const std::string& proto,
                                  analyzer::proto::ResultsList* results_list) {
  if (!results_list->ParseFromString(proto)) {
    LOG(ERROR) << "Failed to parse ResultsList";
    return absl::InvalidArgumentError("Failed to parse ResultsList");
  }
  return absl::OkStatus();
}

}  // namespace proto_util
}  // namespace misra

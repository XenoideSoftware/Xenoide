
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

namespace xenoide {

class FileSearchDialogModel {
public:
  virtual ~FileSearchDialogModel();

  virtual std::vector<std::filesystem::path> searchFilePattern(const std::string &filePattern, const int maxResults) = 0;

public:
  static std::unique_ptr<FileSearchDialogModel> create(const std::filesystem::path &basePath);
};
}


#pragma once 

#include "Model.h"

namespace Xenoide {
	using FileSystemVisitor = std::function<bool (const Path &path)>;

	class FileSystemService {
	public:
		virtual ~FileSystemService();

		virtual bool exists(const Path &path) const = 0;

		virtual void enumerate(const Folder& folder, FileSystemVisitor visitor) = 0;

		virtual std::vector<Path> enumerate(const Folder& folder) = 0;

		virtual std::string extractName(const Path& path) const = 0;
	};
}

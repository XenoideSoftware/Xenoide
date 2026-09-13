
#include <xenoide/core/FileSystemServiceBoost.h>

#include <boost/filesystem.hpp>

namespace Xenoide {
	FileSystemServiceBoost::~FileSystemServiceBoost() {}


	bool FileSystemServiceBoost::exists(const Path& path) const {
		return boost::filesystem::exists(
			boost::filesystem::path(path.value)
		);
	}


	void FileSystemServiceBoost::enumerate(const Folder& folder, FileSystemVisitor visitor) {
		// using boost::filesystem::recursive_directory_iterator;
		using boost::filesystem::directory_iterator;
		using boost::filesystem::is_directory;
		using boost::filesystem::path;

		directory_iterator current{folder.path}, end;

		while (current != end) {
			const path currentPath = current->path();

			const bool continue_ = visitor({
				is_directory(currentPath) ? PathType::Folder : PathType::File,
				currentPath.string()
			});

			if (!continue_) {
				break;
			}

			++current;
		}
	}


	std::vector<Path> FileSystemServiceBoost::enumerate(const Folder& folder) {
		std::vector<Path> children;

	// using boost::filesystem::recursive_directory_iterator;
		using boost::filesystem::directory_iterator;
		using boost::filesystem::is_directory;
		using boost::filesystem::path;

		directory_iterator current{folder.path}, end;

		while (current != end) {
			const path currentPath = current->path();

			children.push_back({
				is_directory(currentPath) ? PathType::Folder : PathType::File,
				currentPath.string()
			});

			++current;
		}

		return children;
	}


	std::string FileSystemServiceBoost::extractName(const Path& path) const {
		return boost::filesystem::path(path.value).filename().string();
	}
}

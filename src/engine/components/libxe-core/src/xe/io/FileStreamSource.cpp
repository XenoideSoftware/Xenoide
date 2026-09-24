
#include <xe/io/FileStreamSource.h>

#include <fstream>
#include <xe/io/FileStream.h>

namespace xe {
    FileStreamSource::FileStreamSource(const std::string &directory) : directory(directory) {
        /*
        fs::path path;

        if (directory == "") {
            path = fs::current_path();
        } else  {
            path = fs::path(directory);
        }

        if (!fs::exists(path)) {
            if (path.is_relative()) {
                const std::string msg = "FileStreamSource: The directory '" + path.string() + "' doesn't exist.";
                throw std::runtime_error(msg);
            } else {
                const std::string msg =
                "FileStreamSource: The directory '" + directory + "' doesn't exist "
                    + "(current path: " + fs::current_path().string() + ").";

                throw std::runtime_error(msg);
            }
        }
        */
    }

    std::unique_ptr<Stream> FileStreamSource::open(const std::string &path) {
        const std::string finalPath = directory + "/" + path;

        return std::make_unique<FileStream>(finalPath, StreamFlags::Readable);
    }

    bool FileStreamSource::exists(const std::string &path) const {
        const std::string filePath = directory + "/" + path;
        std::fstream fs;

        fs.open(filePath.c_str(), std::ios_base::in);

        return fs.is_open();
    }
} // namespace xe

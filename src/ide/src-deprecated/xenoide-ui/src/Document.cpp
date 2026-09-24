
#include <xenoide/ui/Document.h>

#include <xenoide/core/OS.h>
#include <xenoide/core/FileService.h>
#include <xenoide/ui/DialogManager.h>

namespace xenoide {
    DocumentConfig DocumentConfig::Default() {
        return {"Consolas", 10, 4, true, true};
    }

    Document::~Document() {
    }
} // namespace xenoide

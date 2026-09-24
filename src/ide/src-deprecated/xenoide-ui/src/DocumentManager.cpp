
#include <xenoide/ui/DocumentManager.h>

#include <list>
#include <algorithm>
#include <iostream>
#include <xenoide/ui/DialogManager.h>

namespace xenoide {
    DocumentManager::DocumentManager(DocumentManagerPresenter *presenter) {
        this->presenter = presenter;
    }

    DocumentManager::~DocumentManager() {
    }

    std::vector<Document *> DocumentManager::enumerateDocuments() {
        std::vector<Document *> editors;

        editors.reserve(this->getDocumentCount());

        for (int i = 0; i < this->getDocumentCount(); i++) {
            editors.push_back(this->getDocument(i));
        }

        return editors;
    }

    void DocumentManager::closeDocuments(const std::vector<Document *> &documents) {
        for (Document *document : documents) {
            this->closeDocument(document);
        }
    }
} // namespace xenoide


#include <xenoide/ui/DocumentManagerModel.h>

#include <list>
#include <algorithm>
#include <iostream>
#include <xenoide/ui/DialogManager.h>
#include <xenoide/ui/DocumentModel.h>

namespace xenoide {
    DocumentManagerModel::~DocumentManagerModel() {
    }

    DocumentModel *DocumentManagerModel::createDocument() {
        auto document = std::make_unique<DocumentModel>(++createdDocumentCount);
        auto documentPtr = document.get();

        documents.push_back(std::move(document));

        return documentPtr;
    }

    DocumentModel *DocumentManagerModel::createDocument(const std::filesystem::path &filePath) {
        auto document = this->createDocument();
        document->setFilePath(filePath.string());

        return document;
    }

    void DocumentManagerModel::closeDocument(DocumentModel *documentModel) {
        auto documentIt = std::find_if(documents.begin(), documents.end(), [documentModel](auto &document) { return document.get() == documentModel; });

        if (documentIt != documents.end()) {
            documents.erase(documentIt);
        }
    }

    std::vector<DocumentModel *> DocumentManagerModel::enumerateDocuments() const {
        std::vector<DocumentModel *> result;

        result.reserve(documents.size());

        for (auto &document : documents) {
            result.push_back(document.get());
        }

        return result;
    }
} // namespace xenoide

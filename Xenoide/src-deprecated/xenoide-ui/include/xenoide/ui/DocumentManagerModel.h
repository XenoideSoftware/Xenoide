
#pragma once

#include "Document.h"

#include <list>
#include <filesystem>

namespace xenoide {
class DocumentModel;
class DocumentManagerModel {
public:
  ~DocumentManagerModel();

  DocumentModel* createDocument();

  DocumentModel* createDocument(const std::filesystem::path &filePath);

  void closeDocument(DocumentModel *documentModel);

  std::vector<DocumentModel*> enumerateDocuments() const;

private:
  std::list<std::unique_ptr<DocumentModel>> documents;
  int createdDocumentCount = 0;
};

}
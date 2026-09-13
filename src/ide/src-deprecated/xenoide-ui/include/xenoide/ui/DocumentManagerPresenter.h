
#pragma once

#include "Document.h"

#include <list>
#include <memory>
#include <filesystem>

namespace xenoide {
  class DocumentManager;
  class DocumentPresenter;
  class DocumentManagerModel;
  class DocumentManagerPresenter {
  public:
    explicit DocumentManagerPresenter(DocumentManagerModel *model);

    ~DocumentManagerPresenter();

    void onInitialized(DocumentManager *view, DialogManager *dialogView);

    void onNewDocument();

    void onOpenDocument(const std::filesystem::path &path);

    void onSaveDocument();

    void onSaveAsDocument();

    void onSaveAllDocuments();

    void onCloseCurrentDocument();

    void onCloseDocument(Document *document);

    void onCloseOtherDocuments(Document *document);

    void onCloseDocumentsToTheRight(Document *document);

    void onCloseAllDocuments();

  private:
    DocumentPresenter* createDocumentMVP();

    DocumentPresenter* createDocumentMVP(const std::filesystem::path &filePath);

    DocumentPresenter* findDocumentPresenter(Document *document);

    DocumentPresenter* findDocumentPresenter(const std::filesystem::path &filePath);

    void closeDocumentPresenter(DocumentPresenter *documentPresenter);

    void closeDocumentMVP(DocumentPresenter *documentPresenter);

  private:
    DialogManager *dialogView = nullptr;
    DocumentManager *view = nullptr;
    DocumentManagerModel *model = nullptr;

    std::list<std::unique_ptr<DocumentPresenter>> documentPresenters;
  };
}

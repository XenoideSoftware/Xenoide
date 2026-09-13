
#include <xenoide/ui/DocumentManagerPresenter.h>

#include <list>
#include <algorithm>
#include <iostream>
#include <xenoide/ui/DialogManager.h>
#include <xenoide/ui/DocumentPresenter.h>
#include <xenoide/ui/DocumentManager.h>
#include <xenoide/ui/DocumentManagerModel.h>

namespace xenoide {
    DocumentManagerPresenter::DocumentManagerPresenter(DocumentManagerModel *model) {
        this->model = model;
    }

    DocumentManagerPresenter::~DocumentManagerPresenter() {}

    void DocumentManagerPresenter::onInitialized(DocumentManager *view, DialogManager *dialogView) {
        this->view = view;
        this->dialogView = dialogView;
    }

    void DocumentManagerPresenter::onNewDocument() {
        this->createDocumentMVP();
    }

    void DocumentManagerPresenter::onOpenDocument(const std::filesystem::path &filePath) {
        DocumentPresenter *documentPresenter;

        if (documentPresenter = this->findDocumentPresenter(filePath)) {
            view->setCurrentDocument(documentPresenter->getView());
        } else {
            this->createDocumentMVP(filePath);
        }
    }

    void DocumentManagerPresenter::onSaveDocument() {
        if (auto document = view->getCurrentDocument()) {
            if (auto documentPresenter = this->findDocumentPresenter(document)) {
                documentPresenter->onSave();
            }
        }
    }

    void DocumentManagerPresenter::onSaveAllDocuments() {
        auto documentViews = view->enumerateDocuments();

        for (auto documentView : documentViews) {
            if (auto documentPresenter = this->findDocumentPresenter(documentView)) {
                if (documentPresenter->onSave() == DocumentPresenter::UserResponse::Cancel) {
                    break;
                }
            }
        }
    }

    void DocumentManagerPresenter::onSaveAsDocument() {
        if (auto document = view->getCurrentDocument()) {
            if (auto documentPresenter = this->findDocumentPresenter(document)) {
                documentPresenter->onSaveAs();
            }
        }
    }

    void DocumentManagerPresenter::onCloseCurrentDocument() {
        this->onCloseDocument(this->view->getCurrentDocument());
    }

    void DocumentManagerPresenter::onCloseDocument(Document *document) {
        std::cout << "DocumentManagerPresenter::onCloseDocument" << std::endl;

        if (auto documentPresenter = this->findDocumentPresenter(document)) {
            if (documentPresenter->onCloseRequested() == DocumentPresenter::UserResponse::Accept) {
                this->closeDocumentMVP(documentPresenter);
            }
        }
    }

    void DocumentManagerPresenter::onCloseOtherDocuments(Document *document) {
        auto documents = view->enumerateDocuments();

        for (auto current : documents) {
            if (current == document) {
                continue;
            }

            if (auto documentPresenter = this->findDocumentPresenter(current)) {
                if (documentPresenter->onCloseRequested() == DocumentPresenter::UserResponse::Accept) {
                    this->closeDocumentMVP(documentPresenter);
                }
            }
        }
    }

    void DocumentManagerPresenter::onCloseDocumentsToTheRight(Document *document) {
        auto documents = view->enumerateDocuments();
        bool close = false;

        for (auto current : documents) {
            if (close) {
                if (auto documentPresenter = this->findDocumentPresenter(current)) {
                    if (documentPresenter->onCloseRequested() == DocumentPresenter::UserResponse::Accept) {
                        this->closeDocumentMVP(documentPresenter);
                    }
                }
            }

            if (current == document) {
                close = true;
            }
        }
    }

    void DocumentManagerPresenter::onCloseAllDocuments() {
        auto documents = view->enumerateDocuments();

        for (auto document : documents) {
            if (auto documentPresenter = this->findDocumentPresenter(document)) {
                if (documentPresenter->onCloseRequested() == DocumentPresenter::UserResponse::Accept) {
                    this->closeDocumentMVP(documentPresenter);
                }
            }
        }
    }

    DocumentPresenter* DocumentManagerPresenter::createDocumentMVP() {
        return this->createDocumentMVP("");
    }

    DocumentPresenter* DocumentManagerPresenter::createDocumentMVP(const std::filesystem::path &filePath) {
        auto documentModel = model->createDocument(filePath);
        auto documentPresenter = new DocumentPresenter(documentModel);
        auto documentView = view->appendDocument(documentPresenter);

        documentPresenters.emplace_back(documentPresenter);

        return documentPresenter;
    }

    DocumentPresenter* DocumentManagerPresenter::findDocumentPresenter(Document *document) {
        auto &dps = documentPresenters;
        auto documentPresenterIt = std::find_if(dps.begin(), dps.end(), [document](std::unique_ptr<DocumentPresenter> &dp) {
            return dp->getView() == document;
        });

        if (documentPresenterIt != dps.end()) {
            return documentPresenterIt->get();
        }

        return nullptr;
    }
    
    DocumentPresenter* DocumentManagerPresenter::findDocumentPresenter(const std::filesystem::path &filePath) {
        auto &dps = documentPresenters;
        auto documentPresenterIt = std::find_if(dps.begin(), dps.end(), [filePath](std::unique_ptr<DocumentPresenter> &dp) {
            return dp->hasFilePath(filePath);
        });

        if (documentPresenterIt != dps.end()) {
            return documentPresenterIt->get();
        }

        return nullptr;
    }

    void DocumentManagerPresenter::closeDocumentPresenter(DocumentPresenter *documentPresenter) {
        auto &dps = documentPresenters;
        auto documentPresenterIt = std::find_if(dps.begin(), dps.end(), [documentPresenter](std::unique_ptr<DocumentPresenter> &dp) {
            return dp.get() == documentPresenter;
        });

        documentPresenters.erase(documentPresenterIt);
    }

    void DocumentManagerPresenter::closeDocumentMVP(DocumentPresenter *documentPresenter) {
        std::cout << "DocumentManagerPresenter::closeDocumentMVP" << std::endl;

        view->closeDocument(documentPresenter->getView());
        model->closeDocument(documentPresenter->getModel());

        this->closeDocumentPresenter(documentPresenter);
    }
}

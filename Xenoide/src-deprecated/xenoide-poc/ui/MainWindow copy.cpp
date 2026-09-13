
#include "MainWindow.hpp"

#include <map>
#include <cassert>
#include <iostream>
#include <functional>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QDockWidget>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

#include "ProjectBrowser.hpp"
#include "DocumentManager.hpp"

#include "felide/Resource.hpp"
#include "felide/Menu.hpp"

namespace felide {

    struct MainWindow::Private {
        std::map<std::string, QAction*> actions;
        std::map<std::string, std::function<void()>> functions;

        const StringResource *stringResource = nullptr;
        MenuBar menuBar;

        ProjectBrowser *projectExplorer = nullptr;
        DocumentManager *documentManager = nullptr;

        Private(const MenuBar menuBar_, const StringResource *stringResource_) : menuBar(menuBar_), stringResource(stringResource) {
            // inicializar las cadenas
            functions = {
                {"file.openfolder", std::bind(&MainWindow::Private::onOpenFolder, this)}
            };
        }

        QMenu* generateMenu(const Menu &menu) {
            const std::string menuTitle = stringResource->get(menu.getKey());

            auto result = new QMenu(QString::fromStdString(menuTitle));

            for (const Menu &child : menu.getChilds()) {
                if (child.isSeparator() == true) {
                    result->addSeparator();

                } else if (child.getChilds().size() > 0) {
                    result->addMenu(this->generateMenu(child));

                } else {
                    const std::string key = child.getKey();
                    const std::string actionName = stringResource->get(key);

                    QAction *action = result->addAction(QString::fromStdString(actionName));

                    actions[key] = action;
                }
            }

            return result;
        }

        QMenuBar* generateMenuBar(const MenuBar &menuBar) {
            auto result = new QMenuBar();

            for (const Menu &menu : menuBar.getMenues()) {
                result->addMenu(this->generateMenu(menu));
            }

            return result;
        }

        void setupMenuBar(MainWindow *window) {
            window->setMenuBar(this->generateMenuBar(menuBar));
        }

        void setupWidgets(MainWindow *window) {
            auto dock = new QDockWidget("Project Explorer", window);

            projectExplorer = new ProjectBrowser(dock);

            dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            dock->setWidget(projectExplorer);

            window->addDockWidget(Qt::LeftDockWidgetArea, dock);

            documentManager = new DocumentManager(window);
            window->setCentralWidget(documentManager);

            // connect inter component signals
            QObject::connect(projectExplorer, &ProjectBrowser::projectItemOpenRequest, [&](const QString &path) {
                QFileInfo info(path);
                documentManager->openDocument(QFileInfo(path).fileName(), path);
            });
        }

        void setupUI(MainWindow *window) {
            assert(window);

            this->setupMenuBar(window);
            this->setupWidgets(window);

            QDesktopWidget desktopWidget;

            const QRect screenSize = desktopWidget.availableGeometry(desktopWidget.primaryScreen());

            window->setMinimumSize(screenSize.size() * 0.4);
            window->resize(screenSize.size() * 0.7);
        }

        void connectSignals() {
            for (const auto &pair : actions) {
                auto functionPos = functions.find(pair.first);

                if (functionPos == functions.end()) {
                    continue;
                }

                QAction *action = pair.second;

                QObject::connect(action, &QAction::triggered, functionPos->second);
            }
        }

        void onOpenFolder() {
            QFileDialog dialog;
            
            dialog.setFileMode(QFileDialog::Directory);
            dialog.setOption(QFileDialog::ShowDirsOnly);

            if (dialog.exec()) {
                const QStringList folders = dialog.selectedFiles();

                assert(folders.size() == 1);

                const QString folder = folders[0];

                projectExplorer->setProjectFolder(folder);
            }
        }
    };

    MainWindow::MainWindow(const MenuBar &menuBar, const StringResource *stringResource) 
        : m_impl(new MainWindow::Private(menuBar, stringResource)) {

        m_impl->setupUI(this);
        m_impl->connectSignals();
    }
    
    MainWindow::~MainWindow() {}
}

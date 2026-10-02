
#include "DialogFactoryImpl.hpp"

#include <QMessageBox>
#include <QFileDialog>

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <numeric>
#include <filesystem>

namespace felide {
    namespace view {
        namespace qt5 {

            class MessageBoxDialog : public Dialog {
            public:
                MessageBoxDialog(const std::string &title, const std::string &msg, DialogIcon icons, DialogButton buttons) {
                    int result = 0;

                    // Buttons
                    QMessageBox::StandardButtons qbuttons;

                    if (buttons == DialogButton::Ok) {
                        qbuttons = QMessageBox::Ok;

                    } else if (buttons == DialogButton::OkCancel) {
                        qbuttons = QMessageBox::Ok | QMessageBox::Cancel;

                    } else if (buttons == DialogButton::YesNoCancel) {
                        qbuttons = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;

                    } else {
                        assert(false);
                    }

                    // Icon
                    if (icons == DialogIcon::Information) {
                        result = QMessageBox::information(nullptr, title.c_str(), msg.c_str(), qbuttons);

                    } else if (icons == DialogIcon::Warning) {
                        result = QMessageBox::warning(nullptr, title.c_str(), msg.c_str(), qbuttons);

                    } else if (icons == DialogIcon::Error) {
                        result = QMessageBox::critical(nullptr, title.c_str(), msg.c_str(), qbuttons);

                    } else if (icons == DialogIcon::Question) {
                        result = QMessageBox::question(nullptr, title.c_str(), msg.c_str(), qbuttons);

                    } else {
                        assert(false);
                    }

                    // Convert result
                    if (result == QMessageBox::Ok) {
                        m_result = DialogResult::Ok;

                    } else if (result == QMessageBox::Cancel) {
                        m_result = DialogResult::Cancel;

                    } else if (result == QMessageBox::Yes) {
                        m_result = DialogResult::Yes;

                    } else if (result == QMessageBox::No) {
                        m_result = DialogResult::No;
                    }
                }

                virtual DialogResult getResult() const override {
                    return m_result;
                }

                virtual DialogData getData() const override {
                    return DialogData();
                }

                virtual DialogDataList getDataList() const override {
                    return DialogDataList();
                }

            private:
                DialogResult m_result;
            };

            enum class QFileDialogType { Open, Save };

            class FileDialog : public Dialog {
            public:
                FileDialog(const std::string &title, const std::vector<Filter> &filters, QFileDialogType dialogType) {
                    QString path = "";

                    const std::string filter = this->buildFilter(filters);

                    std::cout << filter << std::endl;

                    switch (dialogType) {
                    case QFileDialogType::Open:
                        path = QFileDialog::getOpenFileName(nullptr, title.c_str(), "", filter.c_str());
                        break;

                    case QFileDialogType::Save:
                        path = QFileDialog::getSaveFileName(nullptr, title.c_str(), "", filter.c_str());
                        break;
                    }

                    m_result = path != "" ? DialogResult::Ok : DialogResult::Cancel;
                    m_file = path.toStdString();
                }

                virtual DialogResult getResult() const override {
                    return m_result;
                }

                virtual DialogData getData() const override {
                    return DialogData(m_file);
                }

                virtual DialogDataList getDataList() const override {
                    DialogDataList dataList = {DialogData(m_file)};

                    return dataList;
                }

            private:
                std::string buildFilter(const std::vector<Filter> &filters) {

                    std::vector<std::string> filterParts;

                    for (const Filter &filter : filters) {

                        std::vector<std::string> extensions;

                        extensions.resize(filter.extensions.size());

                        std::transform(filter.extensions.begin(), filter.extensions.end(), std::begin(extensions), [](const std::string &ext) { return "*." + ext; });

                        std::stringstream wildcards_ss;
                        for (size_t i = 0; i < extensions.size(); ++i) {
                            wildcards_ss << extensions[i] << (i < extensions.size() - 1 ? " " : "");
                        }
                        std::string wildcards = wildcards_ss.str();

                        filterParts.push_back(filter.desc + "(" + wildcards + ")");
                    }

                    std::stringstream filterParts_ss;
                    for (size_t i = 0; i < filterParts.size(); ++i) {
                        filterParts_ss << filterParts[i] << (i < filterParts.size() - 1 ? ";;" : "");
                    }
                    return filterParts_ss.str();
                }

            private:
                DialogResult m_result;
                std::filesystem::path m_file;
            };

            DialogPtr DialogFactoryImpl::showMessageDialog(const std::string &title, const std::string &msg, DialogIcon icons, DialogButton buttons) const {
                return std::make_unique<MessageBoxDialog>(title, msg, icons, buttons);
            }

            DialogPtr DialogFactoryImpl::showInputDialog(const std::string &title, const std::string &msg) const {
                return DialogPtr();
            }

            DialogPtr DialogFactoryImpl::showFileOpenDialog(const std::string &title, const std::vector<Filter> &filters) const {
                return std::make_unique<FileDialog>(title, filters, QFileDialogType::Open);
            }

            DialogPtr DialogFactoryImpl::showFileSaveDialog(const std::string &title, const std::vector<Filter> &filters) const {
                return std::make_unique<FileDialog>(title, filters, QFileDialogType::Save);
            }
        } // namespace qt5
    } // namespace view
} // namespace felide

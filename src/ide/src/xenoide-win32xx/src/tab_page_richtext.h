#pragma once

#include <wxx_wincore.h>
#include <wxx_richedit.h>
#include <wxx_docking.h>
#include <wxx_gdi.h>

class RichEditView : public CRichEdit {
public:
    RichEditView() = default;
    ~RichEditView() override = default;

    void SetBody(LPCWSTR text);

protected:
    void OnAttach() override;

private:
    RichEditView(const RichEditView &) = delete;
    RichEditView &operator=(const RichEditView &) = delete;

    CFont m_font;
    CString m_body;
};

class OutputContainer : public CDockContainer {
public:
    OutputContainer();
    ~OutputContainer() override = default;

private:
    OutputContainer(const OutputContainer &) = delete;
    OutputContainer &operator=(const OutputContainer &) = delete;

    RichEditView m_view;
};

class LogsContainer : public CDockContainer {
public:
    LogsContainer();
    ~LogsContainer() override = default;

private:
    LogsContainer(const LogsContainer &) = delete;
    LogsContainer &operator=(const LogsContainer &) = delete;

    RichEditView m_view;
};

class OutputTabDocker : public CDocker {
public:
    OutputTabDocker();
    ~OutputTabDocker() override = default;

private:
    OutputTabDocker(const OutputTabDocker &) = delete;
    OutputTabDocker &operator=(const OutputTabDocker &) = delete;

    OutputContainer m_container;
};

class LogsTabDocker : public CDocker {
public:
    LogsTabDocker();
    ~LogsTabDocker() override = default;

private:
    LogsTabDocker(const LogsTabDocker &) = delete;
    LogsTabDocker &operator=(const LogsTabDocker &) = delete;

    LogsContainer m_container;
};

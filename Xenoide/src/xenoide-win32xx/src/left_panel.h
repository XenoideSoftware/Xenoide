#pragma once

#include <wxx_wincore.h>
#include <wxx_treeview.h>
#include <wxx_docking.h>

class FilesView : public CTreeView {
public:
    FilesView() = default;
    ~FilesView() override = default;

protected:
    void OnAttach() override;

private:
    FilesView(const FilesView&) = delete;
    FilesView& operator=(const FilesView&) = delete;
};

class OutlineView : public CTreeView {
public:
    OutlineView() = default;
    ~OutlineView() override = default;

protected:
    void OnAttach() override;

private:
    OutlineView(const OutlineView&) = delete;
    OutlineView& operator=(const OutlineView&) = delete;
};

class FilesContainer : public CDockContainer {
public:
    FilesContainer();
    ~FilesContainer() override = default;

private:
    FilesContainer(const FilesContainer&) = delete;
    FilesContainer& operator=(const FilesContainer&) = delete;

    FilesView m_view;
};

class OutlineContainer : public CDockContainer {
public:
    OutlineContainer();
    ~OutlineContainer() override = default;

private:
    OutlineContainer(const OutlineContainer&) = delete;
    OutlineContainer& operator=(const OutlineContainer&) = delete;

    OutlineView m_view;
};

class FilesTabDocker : public CDocker {
public:
    FilesTabDocker();
    ~FilesTabDocker() override = default;

private:
    FilesTabDocker(const FilesTabDocker&) = delete;
    FilesTabDocker& operator=(const FilesTabDocker&) = delete;

    FilesContainer m_container;
};

class OutlineTabDocker : public CDocker {
public:
    OutlineTabDocker();
    ~OutlineTabDocker() override = default;

private:
    OutlineTabDocker(const OutlineTabDocker&) = delete;
    OutlineTabDocker& operator=(const OutlineTabDocker&) = delete;

    OutlineContainer m_container;
};

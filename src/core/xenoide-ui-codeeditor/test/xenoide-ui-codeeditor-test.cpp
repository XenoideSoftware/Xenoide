#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <gsl-lite/gsl-lite.hpp>

#include "xenoide/CodeEditor.h"
#include "xenoide/CodeEditorPresenter.h"
#include "xenoide/core/FileService.h"

// ---------------------------------------------------------------------------
// Test doubles
// ---------------------------------------------------------------------------

/**
 * Manual fake for the view interface.
 * Stores whatever the presenter last set so tests can read it back.
 * We deliberately do not use a mock here: we care about state, not interaction.
 */
class FakeCodeEditor final : public xenoide::CodeEditor {
public:
    void setContent(const std::string &content) override {
        m_content = content;
    }
    [[nodiscard]] std::string getContent() const override {
        return m_content;
    }

    void setTitle(const std::string &title) override {
        m_title = title;
    }
    [[nodiscard]] std::string getTitle() const override {
        return m_title;
    }

private:
    std::string m_content;
    std::string m_title;
};

/**
 * Hand-written fake for FileService.
 * - load() returns whatever was preloaded into loadReturns (default: empty).
 * - save() records every (path, content) pair into saveCalls.
 * Tests verify interactions by reading back loadCalls / saveCalls.
 */
class FakeFileService : public xenoide::FileService {
public:
    std::map<std::string, std::string> loadReturns;
    std::vector<std::string> loadCalls;
    std::vector<std::pair<std::string, std::string>> saveCalls;

    std::string load(const std::string &filePath) override {
        loadCalls.push_back(filePath);
        const auto it = loadReturns.find(filePath);
        return it != loadReturns.end() ? it->second : std::string{};
    }

    void save(const std::string &filePath, const std::string &content) override {
        saveCalls.emplace_back(filePath, content);
    }
};

// ---------------------------------------------------------------------------
// Shared constants
// ---------------------------------------------------------------------------

static const std::filesystem::path kFilePath{"/tmp/hello.cpp"};
static const std::string kFilePathString = kFilePath.string();
static const std::string kFileContent{"int main() { return 0; }"};

// ---------------------------------------------------------------------------
// Fixture - unsaved (in-memory only) document
// ---------------------------------------------------------------------------

struct UnsavedFilePresenterFixture {
    FakeFileService fs;
    FakeCodeEditor view;
    xenoide::CodeEditorPresenter presenter{gsl_lite::not_null<xenoide::FileService *>(&fs)};

    UnsavedFilePresenterFixture() {
        presenter.onInitialized(gsl_lite::not_null<xenoide::CodeEditor *>(&view));
    }
};

TEST_CASE_METHOD(UnsavedFilePresenterFixture, "Title is Untitled after initialization", "[codeeditor][unsaved]") {
    REQUIRE(view.getTitle() == "Untitled");
}

TEST_CASE_METHOD(UnsavedFilePresenterFixture, "Not modified after initialization", "[codeeditor][unsaved]") {
    REQUIRE_FALSE(presenter.isModified());
}

TEST_CASE_METHOD(UnsavedFilePresenterFixture, "Has no file path after initialization", "[codeeditor][unsaved]") {
    REQUIRE_FALSE(presenter.hasFilePath());
}

TEST_CASE_METHOD(UnsavedFilePresenterFixture, "Content changed sets modified flag", "[codeeditor][unsaved]") {
    presenter.onContentChanged();
    REQUIRE(presenter.isModified());
}

TEST_CASE_METHOD(UnsavedFilePresenterFixture, "Content changed prefixes title with dirty marker", "[codeeditor][unsaved]") {
    presenter.onContentChanged();
    REQUIRE(view.getTitle() == "[*]Untitled");
}

TEST_CASE_METHOD(UnsavedFilePresenterFixture, "Save without file path does not call FileService", "[codeeditor][unsaved]") {
    presenter.onContentChanged();
    presenter.onSave();
    REQUIRE(fs.saveCalls.empty());
}

TEST_CASE_METHOD(UnsavedFilePresenterFixture, "Save without file path leaves document modified", "[codeeditor][unsaved]") {
    presenter.onContentChanged();
    presenter.onSave();
    REQUIRE(presenter.isModified());
}

TEST_CASE_METHOD(UnsavedFilePresenterFixture, "New resets content to empty", "[codeeditor][unsaved]") {
    presenter.onNew();
    REQUIRE(view.getContent().empty());
}

TEST_CASE_METHOD(UnsavedFilePresenterFixture, "New resets title to Untitled", "[codeeditor][unsaved]") {
    presenter.onContentChanged();
    presenter.onNew();
    REQUIRE(view.getTitle() == "Untitled");
}

TEST_CASE_METHOD(UnsavedFilePresenterFixture, "New clears modified flag", "[codeeditor][unsaved]") {
    presenter.onContentChanged();
    presenter.onNew();
    REQUIRE_FALSE(presenter.isModified());
}

TEST_CASE_METHOD(UnsavedFilePresenterFixture, "SaveAs assigns file path", "[codeeditor][unsaved]") {
    presenter.onSaveAs(kFilePath);
    REQUIRE(presenter.hasFilePath());
    REQUIRE(presenter.getFilePath() == kFilePath);
}

TEST_CASE_METHOD(UnsavedFilePresenterFixture, "SaveAs calls FileService save once", "[codeeditor][unsaved]") {
    presenter.onSaveAs(kFilePath);
    REQUIRE(fs.saveCalls.size() == 1);
    REQUIRE(fs.saveCalls[0].first == kFilePathString);
}

TEST_CASE_METHOD(UnsavedFilePresenterFixture, "SaveAs persists current view content", "[codeeditor][unsaved]") {
    view.setContent(kFileContent);
    presenter.onSaveAs(kFilePath);
    REQUIRE(fs.saveCalls.size() == 1);
    REQUIRE(fs.saveCalls[0].first == kFilePathString);
    REQUIRE(fs.saveCalls[0].second == kFileContent);
}

TEST_CASE_METHOD(UnsavedFilePresenterFixture, "SaveAs clears modified flag", "[codeeditor][unsaved]") {
    presenter.onContentChanged();
    presenter.onSaveAs(kFilePath);
    REQUIRE_FALSE(presenter.isModified());
}

TEST_CASE_METHOD(UnsavedFilePresenterFixture, "SaveAs updates title to plain filename", "[codeeditor][unsaved]") {
    presenter.onSaveAs(kFilePath);
    REQUIRE(view.getTitle() == kFilePath.filename().string());
}

// ---------------------------------------------------------------------------
// Fixture - file already saved to disk
// ---------------------------------------------------------------------------

struct SavedFilePresenterFixture {
    FakeFileService fs;
    FakeCodeEditor view;
    xenoide::CodeEditorPresenter presenter{gsl_lite::not_null<xenoide::FileService *>(&fs)};

    SavedFilePresenterFixture() {
        presenter.onInitialized(gsl_lite::not_null<xenoide::CodeEditor *>(&view));
        fs.loadReturns[kFilePathString] = kFileContent;
        presenter.openFile(kFilePath);
    }
};

TEST_CASE_METHOD(SavedFilePresenterFixture, "OpenFile loads content into view", "[codeeditor][saved]") {
    REQUIRE(view.getContent() == kFileContent);
}

TEST_CASE_METHOD(SavedFilePresenterFixture, "OpenFile sets title to filename", "[codeeditor][saved]") {
    REQUIRE(view.getTitle() == kFilePath.filename().string());
}

TEST_CASE_METHOD(SavedFilePresenterFixture, "OpenFile marks document as not modified", "[codeeditor][saved]") {
    REQUIRE_FALSE(presenter.isModified());
}

TEST_CASE_METHOD(SavedFilePresenterFixture, "OpenFile records file path", "[codeeditor][saved]") {
    REQUIRE(presenter.hasFilePath());
    REQUIRE(presenter.getFilePath() == kFilePath);
}

TEST_CASE_METHOD(SavedFilePresenterFixture, "Content changed after open sets modified flag", "[codeeditor][saved]") {
    presenter.onContentChanged();
    REQUIRE(presenter.isModified());
}

TEST_CASE_METHOD(SavedFilePresenterFixture, "Content changed after open prefixes title with dirty marker", "[codeeditor][saved]") {
    presenter.onContentChanged();
    REQUIRE(view.getTitle() == "[*]" + kFilePath.filename().string());
}

TEST_CASE_METHOD(SavedFilePresenterFixture, "Save after edit persists updated content", "[codeeditor][saved]") {
    const std::string updated = "// updated";
    view.setContent(updated);
    presenter.onContentChanged();

    const size_t savesBefore = fs.saveCalls.size();
    presenter.onSave();
    REQUIRE(fs.saveCalls.size() == savesBefore + 1);
    REQUIRE(fs.saveCalls.back().first == kFilePathString);
    REQUIRE(fs.saveCalls.back().second == updated);
}

TEST_CASE_METHOD(SavedFilePresenterFixture, "Save after edit clears modified flag", "[codeeditor][saved]") {
    view.setContent("// edited");
    presenter.onContentChanged();
    presenter.onSave();
    REQUIRE_FALSE(presenter.isModified());
}

TEST_CASE_METHOD(SavedFilePresenterFixture, "Save after edit restores plain filename title", "[codeeditor][saved]") {
    presenter.onContentChanged();
    presenter.onSave();
    REQUIRE(view.getTitle() == kFilePath.filename().string());
}

TEST_CASE_METHOD(SavedFilePresenterFixture, "New after open clears file path", "[codeeditor][saved]") {
    presenter.onNew();
    REQUIRE_FALSE(presenter.hasFilePath());
}

TEST_CASE_METHOD(SavedFilePresenterFixture, "New after open resets title to Untitled", "[codeeditor][saved]") {
    presenter.onNew();
    REQUIRE(view.getTitle() == "Untitled");
}

TEST_CASE_METHOD(SavedFilePresenterFixture, "SaveAs redirects saves to new path", "[codeeditor][saved]") {
    const std::filesystem::path newPath{"/tmp/copy.cpp"};
    const size_t savesBefore = fs.saveCalls.size();
    presenter.onSaveAs(newPath);
    REQUIRE(fs.saveCalls.size() == savesBefore + 1);
    REQUIRE(fs.saveCalls.back().first == newPath.string());
    REQUIRE(presenter.getFilePath() == newPath);
}

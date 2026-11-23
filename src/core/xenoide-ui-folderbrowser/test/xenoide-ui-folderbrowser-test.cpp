#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include <gsl-lite/gsl-lite.hpp>

#include "xenoide/FolderBrowser.h"
#include "xenoide/FolderBrowserPresenter.h"
#include "xenoide/core/FileService.h"
#include "xenoide/core/Model.h"

// ---------------------------------------------------------------------------
// Test doubles
// ---------------------------------------------------------------------------

/**
 * State-based fake for the FolderBrowser view.
 * Records every call made by the presenter so tests can assert on it
 * without worrying about interaction order or call counts.
 */
class FakeFolderBrowser final : public xenoide::FolderBrowser {
public:
    struct RootItem {
        int id;
        std::string name;
        std::string path;
    };

    struct ChildItem {
        int parentId;
        int childId;
        std::string name;
        std::string path;
        bool isFolder;
    };

    void clearItems() override {
        ++clearCount;
        rootItems.clear();
        children.clear();
        expandedIds.clear();
    }

    void addRootItem(int itemId, const std::string &name, const std::string &path) override {
        rootItems.push_back({itemId, name, path});
    }

    void addChildItem(int parentId, int childId, const std::string &name, const std::string &path, bool isFolder) override {
        children.push_back({parentId, childId, name, path, isFolder});
    }

    void expandItem(int itemId) override {
        expandedIds.push_back(itemId);
    }

    // -- Inspection helpers --
    int clearCount = 0;
    std::vector<RootItem> rootItems;
    std::vector<ChildItem> children;
    std::vector<int> expandedIds;
};

/**
 * Hand-written fake for FileService.
 * Only the methods called by FolderExplorer are overridden:
 *  - extractName(): resolved against extractNameReturns (default: empty string).
 *  - enumerate():   resolved against enumerateReturns keyed by folder.path
 *                   (default: empty vector).
 */
class FakeFileService : public xenoide::FileService {
public:
    std::map<xenoide::Path, std::string> extractNameReturns;
    std::map<std::string, std::vector<xenoide::Path>> enumerateReturns;

    std::string extractName(const xenoide::Path &path) const override {
        const auto it = extractNameReturns.find(path);
        return it != extractNameReturns.end() ? it->second : std::string{};
    }

    std::vector<xenoide::Path> enumerate(const xenoide::Folder &folder) override {
        const auto it = enumerateReturns.find(folder.path);
        return it != enumerateReturns.end() ? it->second : std::vector<xenoide::Path>{};
    }
};

// ---------------------------------------------------------------------------
// Shared test data
// ---------------------------------------------------------------------------

static const std::filesystem::path kFolderPath{"/tmp/project"};
static const std::string kFolderPathStr{"/tmp/project"};

// Two immediate children: one sub-folder and one file.
static const xenoide::Path kSrcDir = xenoide::Path::folder("/tmp/project/src");
static const xenoide::Path kReadme = xenoide::Path::file("/tmp/project/README.md");

// One grandchild inside src/.
static const xenoide::Path kMainCpp = xenoide::Path::file("/tmp/project/src/main.cpp");

// ---------------------------------------------------------------------------
// Fixture helpers
// ---------------------------------------------------------------------------

/** Wire common stubbed returns for a successful openFolder("/tmp/project") call. */
static void wireOpenFolder(FakeFileService &fs) {
    fs.extractNameReturns[xenoide::Path::folder(kFolderPathStr)] = "project";
    fs.extractNameReturns[kSrcDir] = "src";
    fs.extractNameReturns[kReadme] = "README.md";
    fs.enumerateReturns[kFolderPathStr] = {kSrcDir, kReadme};
}

// ---------------------------------------------------------------------------
// Fixture - presenter with no folder open yet
// ---------------------------------------------------------------------------

struct NoFolderPresenterFixture {
    FakeFileService fs;
    FakeFolderBrowser view;
    xenoide::FolderBrowserPresenter presenter{gsl_lite::not_null<xenoide::FileService *>(&fs)};

    NoFolderPresenterFixture() {
        presenter.onInitialized(gsl_lite::not_null<xenoide::FolderBrowser *>(&view));
    }
};

TEST_CASE_METHOD(NoFolderPresenterFixture, "Has no folder initially", "[folderbrowser][nofolder]") {
    REQUIRE_FALSE(presenter.hasFolder());
}

TEST_CASE_METHOD(NoFolderPresenterFixture, "GetFolder returns empty path initially", "[folderbrowser][nofolder]") {
    REQUIRE(presenter.getFolder().empty());
}

// ---------------------------------------------------------------------------
// Fixture - presenter after openFolder
// ---------------------------------------------------------------------------

struct OpenFolderPresenterFixture {
    FakeFileService fs;
    FakeFolderBrowser view;
    xenoide::FolderBrowserPresenter presenter{gsl_lite::not_null<xenoide::FileService *>(&fs)};

    OpenFolderPresenterFixture() {
        wireOpenFolder(fs);
        presenter.onInitialized(gsl_lite::not_null<xenoide::FolderBrowser *>(&view));
        presenter.openFolder(kFolderPath);
    }
};

TEST_CASE_METHOD(OpenFolderPresenterFixture, "Has folder after open", "[folderbrowser][open]") {
    REQUIRE(presenter.hasFolder());
}

TEST_CASE_METHOD(OpenFolderPresenterFixture, "GetFolder returns opened path", "[folderbrowser][open]") {
    REQUIRE(presenter.getFolder() == kFolderPath);
}

TEST_CASE_METHOD(OpenFolderPresenterFixture, "ClearItems is called before populating", "[folderbrowser][open]") {
    REQUIRE(view.clearCount == 1);
}

TEST_CASE_METHOD(OpenFolderPresenterFixture, "Root item is added with folder name", "[folderbrowser][open]") {
    REQUIRE(view.rootItems.size() == 1u);
    REQUIRE(view.rootItems[0].name == "project");
}

TEST_CASE_METHOD(OpenFolderPresenterFixture, "Root item carries full path", "[folderbrowser][open]") {
    REQUIRE(view.rootItems.size() == 1u);
    REQUIRE(view.rootItems[0].path == kFolderPathStr);
}

TEST_CASE_METHOD(OpenFolderPresenterFixture, "Two first-level children are added", "[folderbrowser][open]") {
    REQUIRE(view.children.size() == 2u);
}

TEST_CASE_METHOD(OpenFolderPresenterFixture, "First child is folder with correct name", "[folderbrowser][open]") {
    REQUIRE(view.children.size() >= 1u);
    REQUIRE(view.children[0].name == "src");
    REQUIRE(view.children[0].isFolder);
}

TEST_CASE_METHOD(OpenFolderPresenterFixture, "Second child is file with correct name", "[folderbrowser][open]") {
    REQUIRE(view.children.size() >= 2u);
    REQUIRE(view.children[1].name == "README.md");
    REQUIRE_FALSE(view.children[1].isFolder);
}

TEST_CASE_METHOD(OpenFolderPresenterFixture, "Children carry their full path", "[folderbrowser][open]") {
    REQUIRE(view.children.size() >= 2u);
    REQUIRE(view.children[0].path == kSrcDir.value);
    REQUIRE(view.children[1].path == kReadme.value);
}

TEST_CASE_METHOD(OpenFolderPresenterFixture, "Children are parented to root", "[folderbrowser][open]") {
    const int rootId = view.rootItems[0].id;
    for (const auto &child : view.children) {
        REQUIRE(child.parentId == rootId);
    }
}

TEST_CASE_METHOD(OpenFolderPresenterFixture, "Root item is expanded after children are loaded", "[folderbrowser][open]") {
    REQUIRE(view.expandedIds.size() == 1u);
    REQUIRE(view.expandedIds[0] == view.rootItems[0].id);
}

TEST_CASE_METHOD(OpenFolderPresenterFixture, "Second openFolder clears and rebuilds tree", "[folderbrowser][open]") {
    // Stubs from wireOpenFolder() still apply for the second call.
    presenter.openFolder(kFolderPath);

    REQUIRE(view.clearCount == 2);        // cleared twice total
    REQUIRE(view.rootItems.size() == 1u); // only the latest root remains
}

// ---------------------------------------------------------------------------
// Fixture - presenter after openFolder, then a sub-folder is expanded
// ---------------------------------------------------------------------------

struct ItemExpandedPresenterFixture {
    FakeFileService fs;
    FakeFolderBrowser view;
    xenoide::FolderBrowserPresenter presenter{gsl_lite::not_null<xenoide::FileService *>(&fs)};

    int srcId = -1; // ID assigned by the presenter to the "src" child

    ItemExpandedPresenterFixture() {
        wireOpenFolder(fs);

        // Grandchildren returned when src/ is expanded.
        fs.enumerateReturns[kSrcDir.value] = {kMainCpp};
        fs.extractNameReturns[kMainCpp] = "main.cpp";

        presenter.onInitialized(gsl_lite::not_null<xenoide::FolderBrowser *>(&view));
        presenter.openFolder(kFolderPath);

        // The "src" child is the first element added to view.children.
        REQUIRE_FALSE(view.children.empty());
        srcId = view.children[0].childId;
    }
};

TEST_CASE_METHOD(ItemExpandedPresenterFixture, "Expanding folder child adds its children", "[folderbrowser][expand]") {
    presenter.onItemExpanded(srcId);
    // One grandchild (main.cpp) should have been added.
    REQUIRE(view.children.size() == 3u); // 2 original + 1 grandchild
    REQUIRE(view.children[2].name == "main.cpp");
    REQUIRE_FALSE(view.children[2].isFolder);
}

TEST_CASE_METHOD(ItemExpandedPresenterFixture, "Grandchild is parented to src folder", "[folderbrowser][expand]") {
    presenter.onItemExpanded(srcId);
    REQUIRE(view.children.size() == 3u);
    REQUIRE(view.children[2].parentId == srcId);
}

TEST_CASE_METHOD(ItemExpandedPresenterFixture, "Expanding already populated folder is no-op", "[folderbrowser][expand]") {
    presenter.onItemExpanded(srcId); // first expansion - loads children
    const std::size_t childrenAfterFirst = view.children.size();

    presenter.onItemExpanded(srcId); // second expansion - must not re-add
    REQUIRE(view.children.size() == childrenAfterFirst);
}

TEST_CASE_METHOD(ItemExpandedPresenterFixture, "Expanding file item is no-op", "[folderbrowser][expand]") {
    // The second child is README.md (a file); expanding it must be silently ignored.
    const int readmeId = view.children[1].childId;
    const std::size_t childrenBefore = view.children.size();

    presenter.onItemExpanded(readmeId);

    REQUIRE(view.children.size() == childrenBefore);
}

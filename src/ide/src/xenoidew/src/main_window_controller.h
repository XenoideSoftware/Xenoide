
#pragma once 

#include <string>
#include <vector>
#include <optional>
#include <map>
#include <memory>
#include <cassert>

#include "xenoide/core/FileService.h"
#include "xenoide/core/StringUtil.h"

#include "main_window_model.h"

enum class ShowFileDialog {
	Open,
	Save
};

struct ShowFileDialogOptions {
	std::string filter;
	std::string defaultFile;
};

enum class ShowMessageDialogButton {
	No,
	Yes,
	Cancel
};

struct ShowMessageDialogOptions {
	std::string title;
	std::string prompt;
};

class MainWindowView : public Observer<MainWindowNotification> {
public:
	virtual ~MainWindowView() = default;

	virtual std::optional<std::string> showFileDialog(ShowFileDialog dialog, const ShowFileDialogOptions &options) = 0;

	virtual ShowMessageDialogButton showMessageDialog(const ShowMessageDialogOptions &options) = 0;

	virtual void postQuitMessage() = 0;
};

class MainWindowController {
public:
	MainWindowController();
	
	MainWindowController(MainWindowView *view, const SciEditor &editor);

	void onNewFileCommand();

	void onOpenFileCommand();

	void onSaveFileCommand();

	void onSaveAsFileCommand();

	void onEditorUndoCommand();

	void onEditorRedoCommand();

	void onEditorCutCommand();

	void onEditorCopyCommand();

	void onEditorPasteCommand();

	void onEditorModified();

	void onEditorCharAdded(SCNotification const &notification);

	void onClose();

	MainWindowView *view = nullptr;

	MainWindowModel* getModel() {
		assert(model);
		return model.get();
	}

private:
	std::unique_ptr<MainWindowModel> model;
};

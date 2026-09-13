
#pragma once 

#include <set>
#include <vector>
#include <string>
#include <map>
#include <optional>

#include "observer.h"
#include "subject.h"
#include "sci_editor.h"

#include "xenoide/core/FileService.h"
#include "xenoide/core/StringUtil.h"


struct LanguageConfig {
	int lexer = 0;
	std::string keywords;
	std::vector<LanguageStyle> styles;
	std::string filePatternCaption;
	std::vector<std::string> filePatterns;
};

struct MainWindowNotification {
	bool modifiedFlagChanged = false;
	bool filePathChanged = false;
	bool undoBufferChanged = false;
};

class MainWindowModel : public Subject<MainWindowNotification> {
public:
	explicit MainWindowModel(const SciEditor &editor, const std::optional<std::string> &filePath = {});

	std::string getEditorTitle() const;

	std::optional<std::string> getFilePath() const;

	bool isModified() const;

	bool canSave() const;

	void new_();

	void save(const std::optional<std::string> &newFilePath);

	void load(const std::optional<std::string> &newFilePath);

	std::string getFileFilter() const;

	SciEditor getEditor() {
		return editor;
	}

	const SciEditor getEditor() const {
		return editor;
	}

private:
	void updateLexer();

	std::map<std::string, LanguageConfig>::const_iterator detectLanguage(const std::string &filePath) const;

	SciEditor editor;

	std::optional<std::string> filePath;

	std::map<std::string, LanguageConfig> languageConfigMap;
};

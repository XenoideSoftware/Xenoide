
#pragma once 

#include <optional>
#include <string>
#include <vector>
#include <numeric>

#include <Scintilla.h>
#include <SciLexer.h>
#include <ILexer.h>

struct LanguageStyle {
	int style = 0;
	uint32_t colour = 0;
	bool bold = false;
	bool italic = false;
};


struct SciLexerConfig {
	int lexer = 0;
	std::string keywords;
	std::vector<LanguageStyle> styles;
};

inline uint32_t makeRGB(uint8_t r, uint8_t g, uint8_t b) {
	auto const rr = uint32_t(r);
	auto const gg = uint32_t(g);
	auto const bb = uint32_t(b);

	return rr | (gg << 8) | (bb << 16);
}

template<typename StrType>
StrType join(std::vector<StrType> const &patterns, StrType const &sep) {
	return std::accumulate(patterns.begin(), patterns.end(), StrType{}, 
		[sep](const auto value1, auto value2) {
			if (value1.empty()) {
				return value2;
			}

			return value1 + sep + value2;
		});
}

class SciEditor {
public:
	SciEditor() = default;

	SciEditor(sptr_t scintillaPtr, SciFnDirect scintillaFn);

	void syncLexerConfig(const std::optional<SciLexerConfig> &config);

	void syncMarginLineNumber(int marginIndex, std::optional<int> lineCount = {});

	void clearState();

	void clearAll();

	std::string getText() const;

	void setText(const std::string &text);

	void setSavePoint();

	void undo();

	void redo();

	void cut();

	void copy();

	void paste();

	bool getModify() const;

	bool canUndo() const;

	bool canRedo() const;

	void markUndoPoint();

private:
	sptr_t send(unsigned int iMessage, uptr_t wParam = 0, sptr_t lParam = 0);

	sptr_t send(unsigned int iMessage, uptr_t wParam = 0, sptr_t lParam = 0) const;

	sptr_t scintillaPtr = 0;

	SciFnDirect scintillaFn = nullptr;
};

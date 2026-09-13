
#ifndef __felide_editor_qt5_qdialogfactory_hpp__
#define __felide_editor_qt5_qdialogfactory_hpp__

#include <felide/view/DialogFactory.hpp>

namespace felide { namespace view { namespace qt5 {
    
    class DialogFactoryImpl : public DialogFactory {
    public:
        virtual ~DialogFactoryImpl() {}
        
        virtual DialogPtr showMessageDialog(const std::string &title, const std::string &msg, DialogIcon icons, DialogButton buttons) const override;
		virtual DialogPtr showInputDialog(const std::string &title, const std::string &msg) const override;
		virtual DialogPtr showFileOpenDialog(const std::string &title, const std::vector<Filter> &filters) const override;
		virtual DialogPtr showFileSaveDialog(const std::string &title, const std::vector<Filter> &filters) const override;
    };
}}}

#endif

#ifndef FRAME_H
#define FRAME_H

#include "GamePanel.h"
#include "InfoPanel.h"
#include <wx/wxprec.h>

class Frame : public wxFrame
{
public:
    Frame(const wxString& title);

    GamePanel *m_lp;
    InfoPanel *m_rp;
    wxPanel *m_parent;
    wxStatusBar *statusScore;

};
#endif

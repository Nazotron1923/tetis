#ifndef _MAIN_H
#define _MAIN_H

#include <wx/wx.h>
#include "Frame.h"

class TetrisApp : public wxApp
{
public:
    virtual bool OnInit();

    wxBoxSizer *frame_sizer;
};

#endif

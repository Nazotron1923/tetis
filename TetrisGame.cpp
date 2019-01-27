#include "TetrisGame.hpp"
//#include "Board.hpp"
#include <wx/stattext.h>
#include "Commun.h"
/*
TetrisGame::TetrisGame(wxPanel * parent, wxFrame *fr)
    //: wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(180, 380), wxBORDER_SUNKEN)
    : wxPanel(parent, -1, wxPoint(-1, -1), wxSize(-1, -1), wxBORDER_SUNKEN)
{
    t_parent = parent;
    //wxStatusBar *statusBar = CreateStatusBar();
    //statusBar->SetStatusText(wxT("Score: 0"));
    Board* board = new Board(t_parent, fr);
    board->SetFocus();
    board->Start();

    srand(time(NULL));
}*/

Board::Board(wxPanel* parent_t, wxFrame *fr)
//: wxPanel(parent_t, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
    : wxPanel(parent_t, -1, wxPoint(-1, -1), wxSize(180, 340), wxBORDER_SUNKEN)
{
    timer = new wxTimer(this, 1);
    //statusBar = parent_t->GetStatusBar();
    statusBar = fr->GetStatusBar();
    pieceDoneFalling = false;
    started = false;
    paused = false;
    score = 0;
    curX = 0;
    curY = 0;
    count = 0;
    panel = parent_t;
    next.SetRandomShape();

    Clear();

    Connect(wxEVT_PAINT, wxPaintEventHandler(Board::OnPaint));
    Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(Board::OnKeyDown));
    Connect(wxEVT_TIMER, wxCommandEventHandler(Board::OnTimer));
}

void Board::Reset()
{
    started = true;
    pieceDoneFalling = false;
    paused = false;
    score = 0;

    Clear();

    statusBar->SetStatusText(wxT("Score: 0"));
    MakeNewPiece();
    timer->Start(TIMER_INTERVAL);
}

void Board::Start()
{
    if (paused)
        return;

    Reset();
}

void Board::Pause()
{
    if (!started)
        return;

    paused = !paused;

    if (paused)
    {
        timer->Stop();
        statusBar->SetStatusText(wxT("Game Paused"));
    }
    else
    {
        timer->Start(TIMER_INTERVAL);
        wxString str;
        str.Printf(wxT("Score: %d"), score);
        statusBar->SetStatusText(str);
    }
    Refresh();
}

void Board::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);

    wxSize size = GetClientSize();
    int top = size.GetHeight() - BoardHeight * Height();

    for (int i = 0; i < BoardHeight; i++)
        for (int j = 0; j < BoardWidth; j++)
        {
            PieceShape pieceShape = PieceAt(j, BoardHeight - i - 1);
            if (pieceShape == None)
                continue;
            DrawPieceSquare(dc, j * Width(), top + i * Height(), pieceShape);
        }

    if (current.GetShape() == None)
        return;

    for (int i = 0; i < 4; i++)
    {
        int x = curX + current.x(i);
        int y = curY - current.y(i);
        DrawPieceSquare(dc, x * Width(), top + (BoardHeight - y - 1) * Height(),
                        current.GetShape());
    }
}

void Board::OnKeyDown(wxKeyEvent& event)
{
    int keyCode = event.GetKeyCode();

    switch (keyCode)
    {
        case 'P':
            Pause();
            return;
        case 'R':
            Reset();
            return;
    }

    if (!started || current.GetShape() == None || paused)
    {
        event.Skip();
        return;
    }

    switch (keyCode)
    {
        case WXK_SPACE:
            DropCurrentToBottom();
            break;
        case WXK_UP:
            DoMove(current.RotateLeft(), curX, curY);
            break;
        case WXK_DOWN:
            DropCurrentOneLine();
            break;
        case WXK_LEFT:
            DoMove(current, curX - 1, curY);
            break;
        case WXK_RIGHT:
            DoMove(current, curX + 1, curY);
            break;
        default:
            event.Skip();
    }
}

void Board::OnTimer(wxCommandEvent& event)
{
    if (pieceDoneFalling)
    {
        pieceDoneFalling = false;
        MakeNewPiece();
    }
    else
        DropCurrentOneLine();
}

void Board::Clear()
{
    for (int i = 0; i < BoardHeight * BoardWidth; i++)
        board[i] = None;
}

void Board::DropCurrentToBottom()
{
    int y = curY;
    while (y)
        if (!DoMove(current, curX, --y))
            break;
    PieceHitBottom();
}

void Board::DropCurrentOneLine()
{
    if (!DoMove(current, curX, curY - 1))
        PieceHitBottom();
}

void Board::PieceHitBottom()
{
    for (int i = 0; i < 4; i++)
    {
        int x = curX + current.x(i);
        int y = curY - current.y(i);
        PieceAt(x, y) = current.GetShape();
    }
    ClearFullLines();
    if (!pieceDoneFalling)
        MakeNewPiece();
}

void Board::ClearFullLines()
{
    int lines = 0;

    for (int i = BoardHeight - 1; i >= 0; i--)
    {
        bool lineFull = true;
        for (int j = 0; j < BoardWidth; j++)
            if (PieceAt(j, i) == None)
            {
                lineFull = false;
                break;
            }

        if (!lineFull)
            continue;

        lines++;
        for (int j = i; j < BoardHeight - 1; j++)
            for (int k = 0; k < BoardWidth; k++)
                PieceAt(k, j) = PieceAt(k, j + 1);
    }

    if (!lines)
        return;

    score += lines;
    wxString str;
    str.Printf(wxT("Score: %d"), score);
    statusBar->SetStatusText(str);

    pieceDoneFalling = true;
    current.SetShape(None);
    Refresh();
}

void Board::RandomPiece()
{
    current.SetShape(next.GetShape());
    next.SetRandomShape();

    Commun *comm = (Commun *) panel->GetParent();
    /*for (int i = 0; i < 4; i++)
    {
        int x = next.x(i);
        int y = next.y(i);
        comm->m_rp->piece.DrawPiece(comm->m_rp->piece.dc(panel), x * comm->m_rp->piece.Width(), (comm->m_rp->piece.BoardHeight - y - 1) * comm->m_rp->piece.Height(),
                                    next.GetShape())
    }*/
    comm->m_rp->m_text->SetLabel(wxString::Format(wxT("%d"), next.x(1)));
    //comm->m_rp->piece.SetShape(next.GetShape());
    //comm->m_rp->x11 = next.x(1);
    //comm->m_rp->y11 = next.y(1);
}

void Board::MakeNewPiece()
{
    //current.SetRandomShape();
    RandomPiece();
    curX = BoardWidth / 2;
    curY = BoardHeight - 1 + current.MinY();

    if (!DoMove(current, curX, curY))
    {
        current.SetShape(None);
        timer->Stop();
        started = false;
        statusBar->SetStatusText(wxT("You Lose :("));
    }
    count++;

    //Commun *comm = (Commun *) panel->GetParent();
    //comm->m_rp->m_text->SetLabel(wxString::Format(wxT("%d"), count));
}

bool Board::DoMove(const Piece& piece, int newX, int newY)
{
    for (int i = 0; i < 4; i++)
    {
        int x = newX + piece.x(i);
        int y = newY - piece.y(i);

        if (!InBounds(x, y) || PieceAt(x, y) != None)
            return false;
    }

    current = piece;
    curX = newX;
    curY = newY;

    Refresh();

    return true;
}

void Board::DrawPieceSquare(wxPaintDC& dc, int x, int y, PieceShape pieceShape)
{
    wxPen lightPen(light[int(pieceShape)]);
    lightPen.SetCap(wxCAP_PROJECTING);
    dc.SetPen(lightPen);

    dc.DrawLine(x, y + Height() - 1, x, y);
    dc.DrawLine(x, y, x + Width() - 1, y);

    wxPen darkPen(dark[int(pieceShape)]);
    darkPen.SetCap(wxCAP_PROJECTING);
    dc.SetPen(darkPen);

    dc.DrawLine(x + 1, y + Height() - 1, x + Width() - 1, y + Height() - 1);
    dc.DrawLine(x + Width() - 1, y + Height() - 1, x + Width() - 1, y + 1);

    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(colors[int(pieceShape)]));

    dc.DrawRectangle(x + 1, y + 1, Width() - 2, Height() - 2);
}


RightPanel::RightPanel(wxPanel * parent_t, wxFrame *fr)
        :wxPanel(parent_t, wxID_ANY, wxPoint(-1, 100), wxSize(390, 380), wxBORDER_SUNKEN)
{

    sl1 = new wxStaticLine(this, wxID_ANY, wxPoint(25, 50), wxSize(350,1));
    m_text = new wxStaticText(this, -1, wxT("0"), wxPoint(40, 60));
    //st9 = new wxStaticText(this, -1, wxString::Format(wxT("x..: %d"), this->x11), wxPoint(10, 160));
    //st10 = new wxStaticText(this, -1, wxString::Format(wxT("y..: %d"), this->y11), wxPoint(10, 180));

    sl2 = new wxStaticLine(this, wxID_ANY, wxPoint(25, 90), wxSize(350,1));
}


/*Friends::Friends(wxPanel * parent_t, wxFrame *fr)
        :wxPanel(parent_t, wxID_ANY, wxPoint(-1, 360), wxSize(180, 190), wxBORDER_SUNKEN)
//: wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(250, 380))
{
    //
}*/
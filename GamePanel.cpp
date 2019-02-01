#include "GamePanel.h"
#include "InfoPanel.h"
#include "Frame.h"
#include <wx/stattext.h>

GamePanel::GamePanel(wxPanel* parent_t, wxFrame *fr)
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
    panel = parent_t;
    next.SetRandomShape();
    TIMER_INTERVAL = 500;

    Clear();

    Connect(wxEVT_PAINT, wxPaintEventHandler(GamePanel::OnPaint));
    Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(GamePanel::OnKeyDown));
    Connect(wxEVT_TIMER, wxCommandEventHandler(GamePanel::OnTimer));
}

void GamePanel::Reset()
{
    started = true;
    pieceDoneFalling = false;
    paused = false;
    score = 0;

    Clear();

    statusBar->SetStatusText(wxT("Score: 0"));
    MakeNewPiece();
    timer->Start(this->TIMER_INTERVAL);
}

void GamePanel::Start()
{
    if (paused)
        return;

    Reset();
}

void GamePanel::Pause()
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
        timer->Start(this->TIMER_INTERVAL);
        wxString str;
        str.Printf(wxT("Score: %d"), score);
        statusBar->SetStatusText(str);
    }
    Refresh();
}

void GamePanel::OnPaint(wxPaintEvent& event)
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

void GamePanel::OnKeyDown(wxKeyEvent& event)
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

void GamePanel::OnTimer(wxCommandEvent& event)
{
    if (pieceDoneFalling)
    {
        pieceDoneFalling = false;
        MakeNewPiece();
    }
    else
        DropCurrentOneLine();
}

void GamePanel::Clear()
{
    for (int i = 0; i < BoardHeight * BoardWidth; i++)
        board[i] = None;
}

void GamePanel::DropCurrentToBottom()
{
    int y = curY;
    while (y)
        if (!DoMove(current, curX, --y))
            break;
    PieceHitBottom();
}

void GamePanel::DropCurrentOneLine()
{
    if (!DoMove(current, curX, curY - 1))
        PieceHitBottom();
}

void GamePanel::PieceHitBottom()
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

void GamePanel::ClearFullLines()
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
    this->TIMER_INTERVAL = this->TIMER_INTERVAL - lines*10;
    wxString str;
    str.Printf(wxT("Score: %d"), score);
    statusBar->SetStatusText(str);

    pieceDoneFalling = true;
    current.SetShape(None);
    timer->Start(this->TIMER_INTERVAL);
    Refresh();
}

void GamePanel::RandomPiece()
{
    current.SetShape(next.GetShape());
    next.SetRandomShape();

    Frame *comm = (Frame *) panel->GetParent();

    comm->m_rp->piece.SetShape(None);
    comm->m_rp->ClearPeace();

    comm->m_rp->piece.SetShape(next.GetShape());
    comm->m_rp->ChangePeace();
}

void GamePanel::MakeNewPiece()
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
}

bool GamePanel::DoMove(const Piece& piece, int newX, int newY)
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

void GamePanel::DrawPieceSquare(wxPaintDC& dc, int x, int y, PieceShape pieceShape)
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

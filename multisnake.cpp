#include <iostream>
#include <iomanip>
#include <sstream> // for ostringstream
#include <conio.h>
#include <windows.h>
#include <ctime>


// More info on Windows console programming can be found at
//https://msdn.microsoft.com/en-us/library/windows/desktop/ms683171(v=vs.85).aspx
//https://msdn.microsoft.com/en-us/library/windows/desktop/ms686974(v=vs.85).aspx
//https://msdn.microsoft.com/en-us/library/windows/desktop/ms682073(v=vs.85).aspx
//https://msdn.microsoft.com/en-us/library/windows/desktop/ms685032(v=vs.85).aspx
//https://msdn.microsoft.com/en-us/library/windows/desktop/ms682088(v=vs.85).aspx#_win32_character_attributes

#define USERSNAKEBODY       1
#define CPUSNAKEBODY        2

#define APPLE           0x3
#define BONUSAPPLE      0xE

#define RESETCOLOR      7

#define DELAYTIME       50 // 50 ms is the default delay in the main engine loop

#define DEFAULT_NUM_OF_SNAKES   25
#define DEFAULT_MAX_PINEAPPLES  125
#define MAX_NUM_OF_SNAKES       50
#define MAX_MAX_PINEAPPLES      250

#define ThisIsAnApple(a) (((a == char(APPLE)) || (a == char(BONUSAPPLE))) ? a : 0)

// let's do it counter clockwise, it'll make it easier to generate
// CPUSnakes' movements
typedef enum DirectionsEnum { Up, Left, Down, Right, NumOfDirs } SnakeDir;
typedef enum KeysEnum { U=72, L=75, D=80, R=77, ESC=27 } KeyPress;

COORD ZeroPosition;

int Latency1, Latency2;


class Snake
{
public:
    COORD * pos;
    int len;
    int MaxSnakeLen;
    SnakeDir SnkDir;
    // This is to avoid hitting itself, as long as we don't make
    // the same counterclockwise/clockwise motions 3 times in a row 
    // we're good
    int PrevDir[2];
    WORD * wEatenAttribute;

    Snake()
    {
        SnkDir = (SnakeDir) (rand() % NumOfDirs);
        MaxSnakeLen = 0;
        len = 0;
        
        PrevDir[0] = PrevDir[1] = 0xFF;
    }

    ~Snake()
    {
        delete pos;
        delete wEatenAttribute;
    }

    void SetSnakeBuffer  (int Size);
    void UpdateSnakeBody (COORD * oldhead);
    void UpdateSnakeHead (COORD &Min, COORD &Max);
};
//////////////////////////////////////////////////////////////////////////////////////////////
void Snake::SetSnakeBuffer (int Size)
{
    pos = new COORD[Size];
    wEatenAttribute = new WORD[Size];
    len = 1;
    MaxSnakeLen = Size;
}

void Snake::UpdateSnakeBody(COORD * oldhead)
{
    // update the body by shifting everything by one
    // update the body starting from the end
    // but remember that the head is already updated
    // so handle it carefully

    for (int p = len - 1; p > 0; p--)
    {
        pos[p] = (p > 1 ? pos[p-1] : *oldhead);
    }
}

void Snake::UpdateSnakeHead(COORD &Min, COORD &Max)
{
    if(SnkDir == Right) //If snake is going right
    {
        pos[0].X++;
    }
    else if(SnkDir == Left) //If snake is going left
    {
        pos[0].X--;
    }
    else if(SnkDir == Up) //If snake is going up
    {
        pos[0].Y--; // up is a smaller y coordinate since top is y = 0
    }
    else //If snake is going down(only case left)
    {
        pos[0].Y++;
    }


    //If it hits the border... just teleport to the other side
    if ( pos[0].X < Min.X )
    {
        pos[0].X = Max.X;
    }
    else if ( pos[0].X > Max.X )
    {
        pos[0].X = Min.X;
    }

    if ( pos[0].Y < Min.Y )
    {
        pos[0].Y = Max.Y;
    }
    else if ( pos[0].Y > Max.Y )
    {
        pos[0].Y = Min.Y;
    }
}



 ////////////////////////////////////////////////////////////////////////////
class Board
{

    typedef struct
    {
        COORD   pos;
        WORD    Attrib;
        char    Char;
    }PAS; // [P]ine[A]pple[S]tructure
private:
    int   Apples, Bonus, ExtraApple, Delay, ExtraDelay;
    COORD Size, Max, Min;
    PAS    * pineapple;  // 2 coordinates plus 1 type
    char   * mapmem;         // extra 1 char for NULL to terminate the string
    char   * blankmem;       // extra 1 char for NULL to terminate the string
    char  ** map;                     //Map of game
    char  ** blankmap;                //Map of game
    bool  KB;
    int   MultiPineApple;
    int   NumOfFreeSpace;
    int   YOffset;
    int   NumOfSnakes;
    int   MaxPineapples;
    int   DoubleBuffer;
    int   MaxCpuSnakeLen;
    bool  CycleDisplaySnakes;
    int   ShowSnakeNo;
    unsigned int Last;

    Snake * Snakes;
    char  * Bodies;


    // Graphics related stuffs
    HANDLE hStdout, hNewScreenBuffer;
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
    WORD wOldColorAttrs;
    DWORD cWritten;

    SMALL_RECT srctReadRect;
    SMALL_RECT srctWriteRect;
    COORD coordBufSize;
    COORD coordBufCoord;

    CHAR_INFO * chiBuffer;


public:
    Board()
    {
        ResetCommonVariables();

        NumOfSnakes = DEFAULT_NUM_OF_SNAKES;
        MaxPineapples = DEFAULT_MAX_PINEAPPLES;

        SetImportantBuffers();
    }

    Board(int NumSnakesInput, int MaxPineInput, int DblBfr)
    {
        ResetCommonVariables();

        NumOfSnakes = (NumSnakesInput > MAX_NUM_OF_SNAKES || NumSnakesInput < 1) ? DEFAULT_NUM_OF_SNAKES : NumSnakesInput;
        MaxPineapples = (MaxPineInput > MAX_MAX_PINEAPPLES || MaxPineInput < 1) ? DEFAULT_MAX_PINEAPPLES : MaxPineInput;
        DoubleBuffer = DblBfr;

        SetImportantBuffers();
    }

    ~Board()
    {
        delete mapmem;
        delete blankmem;
        delete map;
        delete blankmap;

        delete chiBuffer;

        delete pineapple;
        delete Snakes;
        delete Bodies;
    }

    void ResetCommonVariables ();
    void SetImportantBuffers ();
    void GetScreenSize ();
    void InitBuffers ();
    void InitSnakes ();
    void ReDrawBoard();
    void MainLoop ();                         // Main Loop
    bool UpdateSnakeMovement(int u, char key);
    void KEYBOARD ();                      // Checking for input
    void Pineapple ();                           // Generating new apples
    void GameOver ();                           // Game over ?.
    void UpdatePineapples (int u);
    bool FindNextDir (int u);
    void Start ();                               // Start -_-
};

 //////////////////////////////////////////////////////////////////////////////////
void Board::ResetCommonVariables()
{
    Last = 0;

    Size.Y = Size.X = Min.Y = Min.X = Max.Y = Max.X = 0;
    YOffset = 1;

    KB = false;

    Apples = 0;
    Bonus = 0;
    ExtraApple = 0; //For bonus apples

    MultiPineApple = 0;

    Delay = DELAYTIME;
    ExtraDelay = 0;

    ZeroPosition.X = 0;
    ZeroPosition.Y = 0;

    DoubleBuffer = 1;

    CycleDisplaySnakes = false;    
    ShowSnakeNo = 1;
}

void Board::SetImportantBuffers()
{
    GetScreenSize();

    InitBuffers();
    InitSnakes();

    NumOfFreeSpace = (Max.Y - Min.Y + 1) * (Max.X - Min.X + 1) - NumOfSnakes;
    MaxPineapples = (MaxPineapples > NumOfFreeSpace) ? NumOfFreeSpace - 1 : MaxPineapples;
    
    MaxCpuSnakeLen = ((Max.Y - Min.Y + 1) < (Max.X - Min.X + 1)) ? (Max.Y - Min.Y + 1) : (Max.X - Min.X + 1);
}

void Board::GetScreenSize()
{
    COORD Position;
    CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;

    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hStdout, &ConsoleScreenBufferInfo);

    // extra -1 for Y for the # of Apples status line
    // the last -1 for Size.Y is for \n at the end of the screen
    Size.Y = (ConsoleScreenBufferInfo.srWindow.Bottom - ConsoleScreenBufferInfo.srWindow.Top + 1) - YOffset - 1;
    Size.X = ConsoleScreenBufferInfo.srWindow.Right - ConsoleScreenBufferInfo.srWindow.Left + 1;

    Min.Y = 1;
    Min.X = 1;

    //#define SIZE_Y  ((Y_MAX - Y_MIN + 1) + 1 + 1) // 1 for top border, 1 for bottom border
    Max.Y = (Size.Y - 1 - 1) - 1 + Min.Y;
    //#define SIZE_X  ((X_MAX - X_MIN + 1) + 1 + 1 + 1) // 1 for left border, 1 for right border and 1 for '\n'
    Max.X = (Size.X - 1 - 1 - 1) - 1 + Min.Y;

    srctReadRect.Top = 0;
    srctReadRect.Left = 0;
    srctReadRect.Bottom = Size.Y + 1 + 1 - 1;
    srctReadRect.Right = Size.X - 1;

    coordBufSize.Y = Size.Y + YOffset + 1;
    coordBufSize.X = Size.X;

    coordBufCoord.X = 0;
    coordBufCoord.Y = 0;

    srctWriteRect.Top = 0;
    srctWriteRect.Left = 0;
    srctWriteRect.Bottom = Size.Y + 1 + 1 - 1;
    srctWriteRect.Right = Size.X - 1;
}

void Board::InitBuffers()
{
    mapmem   = new char[Size.Y * Size.X + 1];
    blankmem = new char[Size.Y * Size.X + 1];

    map      = new char*[Size.Y];
    blankmap = new char*[Size.Y];

    for (int p = 0; p < Size.Y; p++)
    {
        map[p]      = (char *)   &mapmem[p * Size.X];
        blankmap[p] = (char *) &blankmem[p * Size.X];
    }

    chiBuffer = new CHAR_INFO[(Size.Y + YOffset + 1)*Size.X];

    hNewScreenBuffer = CreateConsoleScreenBuffer( GENERIC_READ |     // read/write access
                                                  GENERIC_WRITE,
                                                  FILE_SHARE_READ |
                                                  FILE_SHARE_WRITE,     // shared
                                                  NULL,     // default security attributes
                                                  CONSOLE_TEXTMODE_BUFFER, // must be TEXTMODE
                                                  NULL);
    // reserved; must be NULL
    if (hNewScreenBuffer == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, TEXT("CreateConsoleScreenBuffer failed"), TEXT("Error"), MB_OK);
        hNewScreenBuffer = hStdout;
    }

    pineapple = new PAS[MaxPineapples];
    memset(pineapple, 0, MaxPineapples * sizeof(PAS));
}

void Board::InitSnakes()
{
    Snakes = new Snake[NumOfSnakes];
    Bodies = new char[NumOfSnakes];

    for (int u = 0; u < NumOfSnakes; u++)
    {
        Bodies[u] = (u == 0) ? USERSNAKEBODY : CPUSNAKEBODY;

        Snakes[u].SetSnakeBuffer(Max.Y * Max.X);
        Snakes[u].pos[0].X = (rand() % (Max.X - Min.X + 1)) + 1;
        Snakes[u].pos[0].Y = (rand() % (Max.Y - Min.Y + 1)) + 1;
        Snakes[u].wEatenAttribute[0] = (u == 0) ? (FOREGROUND_RED | FOREGROUND_GREEN) : FOREGROUND_BLUE;
        Snakes[u].wEatenAttribute[0] |= FOREGROUND_INTENSITY;
    }
}

void Board::Start()
{
    for(int y = 0; y < Size.Y; y++)
    {
        for(int x = 0; x < Size.X; x++)
        {
            blankmap[y][x] = ' ';

            if (y == Min.Y - 1 || y == Max.Y + 1)
            {
                blankmap[y][x] = 0xB1;
            }
            if (x == Min.X - 1 || x == Max.X + 1)
            {
                blankmap[y][x] = 0xB2;
            }
            if (x == Max.X + 2)
            {
                blankmap[y][x] = '\n';
            }
        }
    }
    blankmem[Size.Y * Size.X + 1 - 1] = '\0';
    memcpy(mapmem, blankmem, Size.Y * Size.X + 1);

    Pineapple(); //Let's generate a pineapple

    for (int u = NumOfSnakes - 1; u >= 0 ; u--)
    {
        for (int p = 0; p < Snakes[u].len; p++)
        {
            map[Snakes[u].pos[p].Y][Snakes[u].pos[p].X] = Bodies[u]; //Let's make head - block
        }
    }

    for (int q = 0; q < MaxPineapples; q++)
    {
        if (pineapple[q].Char != 0)
            map[pineapple[q].pos.Y][pineapple[q].pos.X] = char(pineapple[q].Char);
    }

    MainLoop();
}
//////////////////////////////////////////////////////////////////////////////////////////
void Board::GameOver()
{
    Sleep(10);
    system("cls");

    std::cout << std::endl << "You've successfully eaten " << Apples + Bonus << " Apples, Yummy!";

    // Delay put in for the cases where the exe is run directly
    // without first starting the shell, thus without a delay
    // it will immediately close without showing the score to the user
    Sleep(800);
    exit(0);
}


void Board::ReDrawBoard()
{
    // recopy the frames
    memcpy(mapmem, blankmem, Size.Y * Size.X + 1);

    // Add pineapples
    for (int q = 0; q < MaxPineapples; q++)
    {
        if (pineapple[q].Char != 0)
            map[pineapple[q].pos.Y][pineapple[q].pos.X] = char(pineapple[q].Char);
    }

    // Add snakes
    int u = (CycleDisplaySnakes) ? ShowSnakeNo : NumOfSnakes - 1;
    int uend = (CycleDisplaySnakes) ? ShowSnakeNo : 0;
    for (; u >= uend ; u--)
    {
        for (int q = 0; q < Snakes[u].len; q++)
        {
            map[Snakes[u].pos[q].Y][Snakes[u].pos[q].X] = Bodies[u];
        }
    }
    
    // Write the usual infos
    std::ostringstream out;
    out << Apples + Bonus << " Apples Eaten ... (" << std::setw(3) << std::left << Delay << ":"<< std::setw(4) << std::left << Latency1 << ":" << std::setw(6) << std::left << Latency2 <<")\n";

    // Redraw/Flush Buffers
    if (DoubleBuffer != 0)
    {
        // Don't forget to reset the cursor position before every re-draw
        SetConsoleCursorPosition(hNewScreenBuffer, ZeroPosition);

        // Write Apple thingy ...
        SetConsoleTextAttribute(hNewScreenBuffer, FOREGROUND_BLUE | BACKGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY);
        WriteFile(
            hNewScreenBuffer,
            out.str().c_str(),
            lstrlenA(out.str().c_str()),
            &cWritten,
            NULL);
        SetConsoleTextAttribute(hNewScreenBuffer, RESETCOLOR);

        // Write the board + pineapples + snakes
        WriteConsole(
            hNewScreenBuffer,
            mapmem,
            lstrlenA(mapmem),
            &cWritten,
            NULL);

        // Copy from the temporary buffer to the new screen buffer.
        // If we don't do this dual buffer thingy the screen will
        // flicker, although it'll make a good flickering effect nonetheless
        ReadConsoleOutput(
           hNewScreenBuffer,        // screen buffer to read from
           chiBuffer,      // buffer to copy into
           coordBufSize,   // col-row size of chiBuffer
           coordBufCoord,  // top left dest. cell in chiBuffer
           &srctReadRect);

        // Set border to bright yellow
        for (int y = YOffset + 1; y <= Max.Y + 1; y++)
        {
            chiBuffer[y * Size.X].Attributes = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
            chiBuffer[y * Size.X + Max.X + 1].Attributes = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
        }
        
        for (int x = 0; x < Max.X - Min.X + 1 + 1 + 1; x++)
        {
            chiBuffer[Size.X + x].Attributes = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
            chiBuffer[(Max.Y + 2) * Size.X + x].Attributes = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
        }
   
        // Set pineapples' colors
        for (int q = 0; q < MaxPineapples; q++)
        {
            if (pineapple[q].Char != 0)
            {
                chiBuffer[(pineapple[q].pos.Y + YOffset) * Size.X + pineapple[q].pos.X].Attributes = pineapple[q].Attrib;
            }
        }

        // Set snakes' colors
        int u = (CycleDisplaySnakes) ? ShowSnakeNo : NumOfSnakes - 1;
        int uend = (CycleDisplaySnakes) ? ShowSnakeNo : 0;
        for (; u >= uend ; u--)
        {
            for (int q = 0; q < Snakes[u].len; q++)
            {
                chiBuffer[(Snakes[u].pos[q].Y + YOffset) * Size.X + Snakes[u].pos[q].X].Attributes = Snakes[u].wEatenAttribute[q];
            }
        }

        WriteConsoleOutput(
            hStdout, // screen buffer to write to
            chiBuffer,        // buffer to copy from
            coordBufSize,     // col-row size of chiBuffer
            coordBufCoord,    // top left src cell in chiBuffer
            &srctWriteRect);  // dest. screen buffer rectangle
    }
    else
    {
        // This is the simple rendering, doesn't support different colors
        // for different characters though
        SetConsoleCursorPosition(hStdout, ZeroPosition);
        SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE | BACKGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY);
        std::cout << out.str();

        //mapmem[Size.Y * Size.X + 1 - 1] = '\0';
        SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE | BACKGROUND_GREEN);
        std::cout << mapmem;
        SetConsoleTextAttribute(hStdout, RESETCOLOR);
    }
}

void Board::UpdatePineapples(int u)
{
    for (int q = 0; q < MaxPineapples; q++)
    {
        if (map[Snakes[u].pos[0].Y][Snakes[u].pos[0].X] == pineapple[q].Char &&
            Snakes[u].pos[0].X == pineapple[q].pos.X &&
            Snakes[u].pos[0].Y == pineapple[q].pos.Y)
        {
            Snakes[u].wEatenAttribute[Snakes[u].len - 1] = pineapple[q].Attrib;
            pineapple[q].Char = 0;
            return;
        }
    }
}

void Board::MainLoop()
{
    while (1)
    {
        if (ExtraApple > 10)
        {
            // Decrease the bonus the longer the user takes
            // to eat the bonus apple
            ExtraApple--;
        }

        // Snake Movement Generator
        KEYBOARD();
        if (KB || (GetTickCount() - Last) >= Delay)
        {
            int StartTime = GetTickCount();
            int PineappleEaten = 0;

            for (int u = 0; u < NumOfSnakes; u++)
            {
                // Don't generate another movement unless it's a CPU Snake
                if (u != 0)
                {
                    // if it fails, try reversing the direction
                    if (FindNextDir(u) == false)
                        FindNextDir(u);
                }

                // save the head position
                COORD oldhead = Snakes[u].pos[0];

                Snakes[u].UpdateSnakeHead(Min, Max);

                // if it hits itself ...
                // GAME OVER, except if it's the tail because by moving the head
                // the tail also moves by one unit

                if ( ( Snakes[u].pos[0].Y != Snakes[u].pos[Snakes[u].len-1].Y ||
                       Snakes[u].pos[0].X != Snakes[u].pos[Snakes[u].len-1].X ) &&
                       map[Snakes[u].pos[0].Y][Snakes[u].pos[0].X] == Bodies[u])
                {
                    if (u == 0)
                    {
                        GameOver();
                    }
                }

                //If it eats a pineapple ...
                int k = ThisIsAnApple(map[Snakes[u].pos[0].Y][Snakes[u].pos[0].X]);
                if (k > 0 && ( u == 0 || (u > 0 && Snakes[u].len < MaxCpuSnakeLen)))
                {
                    Snakes[u].len++;
                    PineappleEaten++;
                    UpdatePineapples(u);

                    Apples++;
                    Bonus = (k > APPLE) ? (Bonus + ExtraApple) : Bonus;
                    
                    // This is to avoid multiple snakes eating the same apple
                    // only one snake can eat the apple
                    // this only happens snce we allow snakes to be on top
                    // of one another, and don't set it to ' ' because
                    // pineapple might then overwrite the new snake head
                    map[Snakes[u].pos[0].Y][Snakes[u].pos[0].X] = 0xFF;
                }

                Snakes[u].UpdateSnakeBody(&oldhead);

                // it fills the whole screen, GAME OVER
                if (Snakes[u].len >= Snakes[u].MaxSnakeLen)
                {
                    GameOver();
                }
            }
            
            MultiPineApple -= PineappleEaten;
            Pineapple();

            // Redraw the whole screen using the bitmap double array
            Latency1 = GetTickCount() - StartTime;
            
            ReDrawBoard();

            Latency2 = GetTickCount() - StartTime - Latency1;

            if (Latency1 + Latency2 < DELAYTIME + ExtraDelay)
                Delay = DELAYTIME - Latency1 - Latency2 + ExtraDelay;
            else
                Delay = 0;

            if (KB == false)
            {
                Last = GetTickCount();
            }

            KB = false;
        }
        //Sleep(1);
    }
}

void Board::Pineapple()
{
    // no need to delete old pineapple since it's already replaced
    // by the new head

    int i = MultiPineApple;

    if (MultiPineApple < MaxPineapples)
    {
        MultiPineApple += rand() % MaxPineapples + 1;

        if (MultiPineApple > MaxPineapples)
        {
            MultiPineApple = MaxPineapples;
        }
    }

    MultiPineApple = (MultiPineApple > NumOfFreeSpace) ? NumOfFreeSpace - 1 : MultiPineApple;

    while (i < MultiPineApple)
    {
        int j = 0;
        
        while (pineapple[j].Char != 0 && j < MaxPineapples)
        {
            j++;
        }
        
        pineapple[j].pos.X = ( rand() % (Max.X - Min.X + 1) ) + 1;
        pineapple[j].pos.Y = ( rand() % (Max.Y - Min.Y + 1) ) + 1;

        // if the new pineapple place is occupied,
        // just find the next available one
        while (map[pineapple[j].pos.Y][pineapple[j].pos.X] != ' ')
        {
            pineapple[j].pos.X = (pineapple[j].pos.X + 1 > Max.X) ? Min.X : pineapple[j].pos.X + 1;
            pineapple[j].pos.Y = (pineapple[j].pos.X == Min.X) ?
                                 ((pineapple[j].pos.Y + 1 > Max.Y) ? Min.Y : pineapple[j].pos.Y + 1) :
                                 pineapple[j].pos.Y;
        }

        //Bonus apple 30% of the time
        int BonusChance = rand() % 10;

        pineapple[j].Char = (BonusChance >= 7) ? char(BONUSAPPLE) : char(APPLE);
        pineapple[j].Attrib = (BonusChance >= 7) ? FOREGROUND_RED : FOREGROUND_GREEN;
        pineapple[j].Attrib |= FOREGROUND_INTENSITY;

        NumOfFreeSpace--;
        
        // this is to avoid adding multiple apples at the same location
        // 0xFF just to make sure it doesn't mess up anything else
        // and since it's constant it'll be faster than pineapple[j].pos.Char
        map[pineapple[j].pos.Y][pineapple[j].pos.X] = 0xFF;

        i++;
    }

    ExtraApple = (rand() % 50) + 50;
}

bool Board::UpdateSnakeMovement(int u, char key)
{
    SnakeDir OldDir = Snakes[u].SnkDir;

    switch( key )
    {
        // up arrow:
        case U:
        {
            Snakes[u].SnkDir = (Snakes[u].SnkDir != Down) ? Up : Snakes[u].SnkDir;
            break;
        }
        // left arrow:
        case L:
        {
            Snakes[u].SnkDir = (Snakes[u].SnkDir != Right) ? Left : Snakes[u].SnkDir;
            break;
        }
        // down arrow:
        case D:
        {
            Snakes[u].SnkDir = (Snakes[u].SnkDir != Up) ? Down : Snakes[u].SnkDir;
            break;
        }
        // right arrow:
        case R:
        {
            Snakes[u].SnkDir = (Snakes[u].SnkDir != Left) ? Right : Snakes[u].SnkDir;
            break;
        }
        // these two keys are used to control speed
        case '+':
        {
            ExtraDelay = (Delay > 0) ? ExtraDelay - 1 : ExtraDelay;
            break;
        }
        case '-':
        {
            ExtraDelay = (ExtraDelay < 100) ? ExtraDelay + 1 : ExtraDelay;
            break;
        }
        // toggle to display User snake only
        case '1':
        {
            CycleDisplaySnakes = true;
            ShowSnakeNo = 0;
            break;
        }
        // Cycle through all the CPU snakes one by one
        case ' ':
        {
            ShowSnakeNo = (CycleDisplaySnakes) ? ((ShowSnakeNo + 1) % NumOfSnakes) : 0;
            CycleDisplaySnakes = true;
            break;
        }
        // get out of the cycling LOL
        case '0':
        {
            CycleDisplaySnakes = false;
            break;
        }
        //case 'esc':
        case ESC:
        {
            GameOver( );
        }
    }

    return (Snakes[u].SnkDir != OldDir) ? true : false;
}

bool Board::FindNextDir(int u)
{
    bool StraightOn = true;
    bool StraightOK = false;
    bool TurnOK = false;
    SnakeDir OldDir = Snakes[u].SnkDir;
    COORD oldhead = Snakes[u].pos[0];
    char keys[NumOfDirs] = {U, L, D, R};
    int CoC;
    
    char key;

    StraightOn = (rand() % 10 <= 7) ? true : false;

    if (StraightOn == true)
    {
        key = keys[(int)Snakes[u].SnkDir];
        
        UpdateSnakeMovement(u, key);
        Snakes[u].UpdateSnakeHead(Min, Max);

        if (map[Snakes[u].pos[0].Y][Snakes[u].pos[0].X] == Bodies[u] ||
            map[Snakes[u].pos[0].Y][Snakes[u].pos[0].X] == Bodies[0])
        {
            Snakes[u].SnkDir = OldDir;
            Snakes[u].pos[0] = oldhead;
        }
        else
        {
            StraightOK = true;
        }
    }
    
    // 80% of the time just go straight
    if (StraightOn == false || StraightOK == false)
    {
        // 1. Choose CC or C
        CoC = (rand() % 2 == 0) ? -1 : 1;

        // 2. Check if it's the 2 prev movement
        bool ConscDir = false;
        if (Snakes[u].PrevDir[0] == Snakes[u].PrevDir[1] && Snakes[u].PrevDir[0] == CoC)
        {
            CoC *= -1;
            ConscDir = true;
        }

        // 3. Check for collision
        int NewDir = (int)(Snakes[u].SnkDir) + CoC;

        NewDir = (NewDir < 0) ? (NumOfDirs - 1) : ((NewDir >= NumOfDirs) ? 0 : NewDir);
        key = keys[NewDir];

        // 4. If collision and not two consecutive directions, flip direction

        UpdateSnakeMovement(u, key);
        Snakes[u].UpdateSnakeHead(Min, Max);

        if (map[Snakes[u].pos[0].Y][Snakes[u].pos[0].X] == Bodies[u] ||
            map[Snakes[u].pos[0].Y][Snakes[u].pos[0].X] == Bodies[0])
        {
            if (ConscDir == false)
            {
                // reset before flipping
                Snakes[u].SnkDir = OldDir;
                Snakes[u].pos[0] = oldhead;
                
                //we need the value of CoC to update PrevDir array
                CoC *= -1;
                NewDir = (int)(Snakes[u].SnkDir) + CoC;
                NewDir = (NewDir < 0) ? (NumOfDirs - 1) : ((NewDir >= NumOfDirs) ? 0 : NewDir);
                key = keys[NewDir];
                
                UpdateSnakeMovement(u, key);
                Snakes[u].UpdateSnakeHead(Min, Max);

                // 5. Check if it is indeed a collision again
                if (map[Snakes[u].pos[0].Y][Snakes[u].pos[0].X] == Bodies[u] ||
                    map[Snakes[u].pos[0].Y][Snakes[u].pos[0].X] == Bodies[0])
                {
                    // if it is a collision, just head straight
                    Snakes[u].SnkDir = OldDir;
                    Snakes[u].pos[0] = oldhead;
                }
                else
                {
                    TurnOK = true;
                }
            }
        }
        else
        {
            TurnOK = true;
        }
    }
    
    // if both CC and C fail and going straight fail, reverse direction
    if (TurnOK == false && StraightOK == false)
    {
        // if all fail, just head in reverse
        // but we need to get the reverse direction, how?
        
        COORD tail, prevtail;
        
        if (Snakes[u].len == 1)
        {
            Snakes[u].SnkDir = (SnakeDir)((int)Snakes[u].SnkDir + 2);
            if (Snakes[u].SnkDir > NumOfDirs)
                Snakes[u].SnkDir = (SnakeDir)0;
        }
        else
        {
            tail = Snakes[u].pos[Snakes[u].len - 1];
            prevtail = Snakes[u].pos[Snakes[u].len - 1 - 1];
            
            int dx = tail.X - prevtail.X;
            int dy = tail.Y - prevtail.Y;
            
            dx = (dx*dx > 1) ? (dx < 1 ? 1 : -1) : dx;
            dy = (dy*dy > 1) ? (dy < 1 ? 1 : -1) : dy;
            
                 if (dy ==  1) Snakes[u].SnkDir = Down;
            else if (dy == -1) Snakes[u].SnkDir = Up;
            else if (dx ==  1) Snakes[u].SnkDir = Right;
            else if (dx == -1) Snakes[u].SnkDir = Left;
        }
        //Snakes[u].SnkDir = OldDir;
        Snakes[u].pos[0] = oldhead;
    
        COORD TempPos;
        WORD TempAttb;
    
        for (int p = 0; p < Snakes[u].len / 2; p++)
        {
            TempPos = Snakes[u].pos[p];
            Snakes[u].pos[p] = Snakes[u].pos[Snakes[u].len - p - 1];
            Snakes[u].pos[Snakes[u].len - p - 1] = TempPos;
            
            TempAttb = Snakes[u].wEatenAttribute[p];
            Snakes[u].wEatenAttribute[p] = Snakes[u].wEatenAttribute[Snakes[u].len - p - 1];
            Snakes[u].wEatenAttribute[Snakes[u].len - p - 1] = TempAttb;
        }
    
        // reset PrevDir
        Snakes[u].PrevDir[0] = Snakes[u].PrevDir[1] = 0xFF;    
    }
    else if (StraightOK == true)
    {
        Snakes[u].pos[0] = oldhead;
        // Do not update CoC if it's straight, otherwise it'll be useless !!!
        // because we're just interested in change of direction not every movement
        //Snakes[u].PrevDir[1] = Snakes[u].PrevDir[0];
        //Snakes[u].PrevDir[0] = 0;
    }
    else if (TurnOK == true)
    {
        Snakes[u].pos[0] = oldhead;
        
        Snakes[u].PrevDir[1] = Snakes[u].PrevDir[0];
        Snakes[u].PrevDir[0] = CoC;
    }

    return (TurnOK || StraightOK);
}

void Board::KEYBOARD(void)
{
    if(_kbhit())
    {
        KB = UpdateSnakeMovement(0, _getch());
    }
}

int main ( int argc, char * argv[] )
{
    if (argc > 4)
    {
        std::cout << "Usage: multisnake NUM_OF_SNAKES MAX_NUM_OF_PINEAPPLES [OPTIONAL: DOUBLE_BUFFER]\n\n";
        std::cout << "NUM_OF_SNAKES        : if it's more than 50, it'll be set to 50, default:25\n";
        std::cout << "MAX_NUM_OF_PINEAPPLES: if it's more than 250, it'll be set to 250, default:125\n";
        std::cout << "DOUBLE_BUFFER        : 0 means OFF, otherwise it's on, default:ON\n";

        return 0;
    }

    int args[3] = {DEFAULT_NUM_OF_SNAKES, DEFAULT_MAX_PINEAPPLES, 1};
    for (int i = 1; i < argc; i++)
    {
        args[i-1] = atoi(argv[i]);
    }

    srand((unsigned)time(0));

    // clear screen for first time use
    system("cls");

    Board MyBoard(args[0], args[1], args[2]);
    MyBoard.Start();
}
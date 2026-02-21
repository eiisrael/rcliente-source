#include "pch.h"
#include <windows.h>
#include <stdio.h>

#include "DebugUI.h"
#include "SControl.h"

// ======================================================
// ESTADO GLOBAL DO INSPECTOR
// ======================================================
static SControl* g_LastPrinted = nullptr;
static bool g_MouseDown = false;
static HANDLE gConsole = GetStdHandle(STD_OUTPUT_HANDLE);

// ======================================================
// FUNÇÃO DE COR (DEVE VIR ANTES DO USO)
// ======================================================
static void SetColor(WORD color)
{
    SetConsoleTextAttribute(gConsole, color);
}

// ======================================================
// PALETA
// ======================================================
#define C_TITLE  (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define C_LABEL  (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY) // ciano
#define C_VALUE  (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE) // branco
#define C_TYPE   (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY) // amarelo
#define C_ON     (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define C_OFF    (FOREGROUND_RED | FOREGROUND_INTENSITY)
#define C_PTR    (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY) // roxo

// ======================================================
// TIPO DO CONTROLE
// ======================================================
static const char* DebugUI_GetTypeName(CONTROL_TYPE type)
{
    switch (type)
    {
    case CONTROL_TYPE::CTRL_TYPE_NONE: return "NONE";
    case CONTROL_TYPE::CTRL_TYPE_CURSOR: return "CURSOR";
    case CONTROL_TYPE::CTRL_TYPE_PANEL: return "PANEL";
    case CONTROL_TYPE::CTRL_TYPE_BUTTON: return "BUTTON";
    case CONTROL_TYPE::CTRL_TYPE_CHECKBOX: return "CHECKBOX";
    case CONTROL_TYPE::CTRL_TYPE_RADIOBUTTON: return "RADIOBUTTON";
    case CONTROL_TYPE::CTRL_TYPE_RADIOBUTTONSET: return "RADIOBUTTONSET";
    case CONTROL_TYPE::CTRL_TYPE_LISTBOX: return "LISTBOX";
    case CONTROL_TYPE::CTRL_TYPE_LISTBOXITEM: return "LISTBOXITEM";
    case CONTROL_TYPE::CTRL_TYPE_MESSAGEBOX: return "MESSAGEBOX";
    case CONTROL_TYPE::CTRL_TYPE_MESSAGEPANEL: return "MESSAGEPANEL";
    case CONTROL_TYPE::CTRL_TYPE_PROGRESSBAR: return "PROGRESSBAR";
    case CONTROL_TYPE::CTRL_TYPE_SCROLLBAR: return "SCROLLBAR";
    case CONTROL_TYPE::CTRL_TYPE_TEXT: return "TEXT";
    case CONTROL_TYPE::CTRL_TYPE_EDITABLETEXT: return "EDITABLETEXT";
    case CONTROL_TYPE::CTRL_TYPE_DIALOG: return "DIALOG";
    case CONTROL_TYPE::CTRL_TYPE_3DOBJ: return "3DOBJ";
    case CONTROL_TYPE::CTRL_TYPE_GRID: return "GRID";
    }

    return "UNKNOWN";
}

// ======================================================
// PRINT PRINCIPAL
// ======================================================
void DebugUI_PrintControl(SControl* ctrl, int mouseX, int mouseY, int localX, int localY)
{
    if (!ctrl)
        return;

    // evita spam segurando mouse
    if (ctrl == g_LastPrinted && g_MouseDown)
        return;

    g_LastPrinted = ctrl;
    g_MouseDown = true;

    system("cls");

    const char* typeName = DebugUI_GetTypeName(ctrl->m_eCtrlType);

    SetColor(C_TITLE);
    printf("============== WYD UI INSPECTOR ==============\n\n");

    SetColor(C_LABEL); printf(" Mouse Screen : "); SetColor(C_VALUE);
    printf("(%d, %d)\n", mouseX, mouseY);

    SetColor(C_LABEL); printf(" Mouse Local  : "); SetColor(C_VALUE);
    printf("(%d, %d)\n\n", localX, localY);

    SetColor(C_LABEL); printf(" ControlID    : "); SetColor(C_VALUE);
    printf("%u\n", ctrl->GetControlID());

    SetColor(C_LABEL); printf(" UniqueID     : "); SetColor(C_VALUE);
    printf("%u\n", ctrl->GetUniqueID());

    SetColor(C_LABEL); printf(" Type         : "); SetColor(C_TYPE);
    printf("%s\n\n", typeName);

    SetColor(C_LABEL); printf(" Position     : "); SetColor(C_VALUE);
    printf("(%.1f , %.1f)\n", ctrl->m_nPosX, ctrl->m_nPosY);

    SetColor(C_LABEL); printf(" Size         : "); SetColor(C_VALUE);
    printf("(%.1f x %.1f)\n\n", ctrl->m_nWidth, ctrl->m_nHeight);

    SetColor(C_LABEL); printf(" Visible      : "); SetColor(ctrl->m_bVisible ? C_ON : C_OFF);
    printf("%d\n", ctrl->m_bVisible);

    SetColor(C_LABEL); printf(" Enabled      : "); SetColor(ctrl->m_bEnable ? C_ON : C_OFF);
    printf("%d\n", ctrl->m_bEnable);

    SetColor(C_LABEL); printf(" Focused      : "); SetColor(ctrl->m_bFocused ? C_ON : C_OFF);
    printf("%d\n", ctrl->m_bFocused);

    SetColor(C_LABEL); printf(" MouseOver    : "); SetColor(ctrl->m_bOver ? C_ON : C_OFF);
    printf("%d\n", ctrl->m_bOver);

    SetColor(C_LABEL); printf(" SelectEnable : "); SetColor(ctrl->m_bSelectEnable ? C_ON : C_OFF);
    printf("%d\n", ctrl->m_bSelectEnable);

    SetColor(C_LABEL); printf(" AlwaysOnTop  : "); SetColor(ctrl->m_bAlwaysOnTop ? C_ON : C_OFF);
    printf("%d\n", ctrl->m_bAlwaysOnTop);

    SetColor(C_LABEL); printf(" Modal        : "); SetColor(ctrl->m_bModal ? C_OFF : C_ON);
    printf("%d\n\n", ctrl->m_bModal);

    SetColor(C_LABEL); printf(" PTR          : "); SetColor(C_PTR);
    printf("%p\n", ctrl);

    SetColor(C_TITLE);
    printf("\n===============================================\n");

    SetColor(C_VALUE);
}

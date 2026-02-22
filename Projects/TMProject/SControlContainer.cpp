#include "pch.h"
#include "TMFieldScene.h"
#include "SControlContainer.h"
#include "SControl.h"
#include "TMGlobal.h"
#include "DebugUI.h"

SControlContainer::SControlContainer(TMScene* pScene) 
	: TreeNode(0)
	, m_pScene(pScene)
{
	m_pFocusControl = nullptr;
	m_pPickedControl = nullptr;

	m_pCursor = new SCursor(0, g_pDevice->m_dwScreenWidth / 2.0f, g_pDevice->m_dwScreenHeight / 2.0f, 32.0f, 32.0f);
	m_pControlRoot = new SControl(0.0f, 0.0f, 0.0f, 0.0f);

	m_bCleanUp = 0;
	m_bInvisibleUI = 0;

	memset(m_pDrawControl, 0, sizeof m_pDrawControl);
	memset(m_pModalControl, 0, sizeof m_pModalControl);
}

SControlContainer::~SControlContainer()
{
	SAFE_DELETE(m_pControlRoot);
	SAFE_DELETE(m_pCursor);
}

int SControlContainer::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (m_pCursor->m_bVisible)
		m_pCursor->OnMouseEvent(dwFlags, wParam, nX, nY);

	int ParentPosX = 0;
	int ParentPosY = 0;
	int bProcessed = 0;

	auto pCurrentControl = m_pControlRoot;
	auto pRootControl = m_pControlRoot;

	for (int i = 0; i < 8; ++i)
	{
		SControl* tmp = m_pModalControl[i];
		if (tmp && tmp->m_bVisible && tmp->m_bModal)
		{
			pCurrentControl = tmp;
			pRootControl = tmp;
			break;
		}
	}

	if (!pCurrentControl)
		return 1;

	// guarda o "melhor alvo" do clique (menor área) pra imprimir no UP
	static SControl* s_bestDown = nullptr;
	static DWORD s_bestDownTime = 0;

	// candidatos do clique atual
	SControl* bestHit = nullptr;
	int bestArea = 0x7FFFFFFF;
	int bestAbsX = 0, bestAbsY = 0;

	do
	{
		if (!pCurrentControl->m_cDeleted && pCurrentControl->m_bVisible)
		{
			int localX = nX - ParentPosX;
			int localY = nY - ParentPosY;

			// captura o menor controle clicado no LBUTTONDOWN
			if (dwFlags == WM_LBUTTONDOWN)
			{
				// ignora o que não é "clicável" (assim o fade panel não ganha)
				if (pCurrentControl->m_bSelectEnable && pCurrentControl->PtInControl(localX, localY))
				{
					int w = (int)pCurrentControl->m_nWidth;
					int h = (int)pCurrentControl->m_nHeight;
					int area = w * h;

					if (area > 0 && area < bestArea)
					{
						bestArea = area;
						bestHit = pCurrentControl;

						// salva a posição absoluta do controle na tela
						bestAbsX = ParentPosX + (int)pCurrentControl->m_nPosX;
						bestAbsY = ParentPosY + (int)pCurrentControl->m_nPosY;
					}
				}
			}

			int ret = pCurrentControl->OnMouseEvent(dwFlags, wParam, localX, localY);

			if (ret == 1)
			{
				int realX = ParentPosX + (int)pCurrentControl->m_nPosX;
				int realY = ParentPosY + (int)pCurrentControl->m_nPosY;
				int realW = (int)pCurrentControl->m_nWidth;
				int realH = (int)pCurrentControl->m_nHeight;

				bool insideScreen =
					realX < (int)g_pDevice->m_dwScreenWidth &&
					realY < (int)g_pDevice->m_dwScreenHeight &&
					realX + realW > 0 &&
					realY + realH > 0;

				if (insideScreen)
					bProcessed = 1;
			}

			if (pCurrentControl->m_pDown)
			{
				ParentPosX += (int)pCurrentControl->m_nPosX;
				ParentPosY += (int)pCurrentControl->m_nPosY;
				pCurrentControl = (SControl*)pCurrentControl->m_pDown;
				continue;
			}
		}

		do
		{
			if (pCurrentControl->m_pNextLink)
			{
				pCurrentControl = (SControl*)pCurrentControl->m_pNextLink;
				break;
			}

			pCurrentControl = (SControl*)pCurrentControl->m_pTop;

			if (!pCurrentControl)
				break;

			ParentPosX -= (int)pCurrentControl->m_nPosX;
			ParentPosY -= (int)pCurrentControl->m_nPosY;

		} while (pCurrentControl != pRootControl);

	} while (pCurrentControl && pCurrentControl != pRootControl);

	// Final do clique: salva/imprime de forma limpa (sem spam)
	if (dwFlags == WM_LBUTTONDOWN)
	{
		s_bestDown = bestHit;
		s_bestDownTime = GetTickCount();
	}
	else if (dwFlags == WM_LBUTTONUP)
	{
		if (s_bestDown)
		{
			DWORD now = GetTickCount();

			// anti-spam: evita repetir em sequência muito rápida
			/*if ((now - s_bestDownTime) < 1500)
			{
				printf("[BEST CLICK] id=%u uid=%u type=%d ptr=%p pos=(%.0f,%.0f) size=(%.0f,%.0f)\n",
					(unsigned)s_bestDown->GetControlID(),
					(unsigned)s_bestDown->GetUniqueID(),
					(int)s_bestDown->GetControlType(),
					s_bestDown,
					s_bestDown->m_nPosX, s_bestDown->m_nPosY,
					s_bestDown->m_nWidth, s_bestDown->m_nHeight
				);
			} */
		}

		s_bestDown = nullptr;
		s_bestDownTime = 0;
	}

	if (!bProcessed)
	{
		auto pPanel = g_pCurrentScene->m_pDescPanel;
		if (pPanel)
			pPanel->SetVisible(0);
	}

	return bProcessed;
}


int SControlContainer::OnKeyDownEvent(unsigned int iKeyCode)
{
	auto pCurrentControl = m_pControlRoot;
	auto pRootControl = m_pControlRoot;

	if (pCurrentControl == nullptr)
		return 0;

	do
	{
		if (!pCurrentControl->m_cDeleted && pCurrentControl->m_bVisible == 1)
		{
			if (pCurrentControl->OnKeyDownEvent(iKeyCode))
				return 1;

			if (pCurrentControl->m_pDown)
			{
				pCurrentControl = static_cast<SControl*>(pCurrentControl->m_pDown);

				continue;
			}
		}

		do
		{
			if (pCurrentControl->m_pNextLink != nullptr)
			{
				pCurrentControl = static_cast<SControl*>(pCurrentControl->m_pNextLink);
				break;
			}

			pCurrentControl = static_cast<SControl*>(pCurrentControl->m_pTop);
		} while (pCurrentControl != pRootControl && pCurrentControl != nullptr);
	} while (pCurrentControl != pRootControl && pCurrentControl != nullptr);

	return 0;
}

// SControlContainer.cpp

int SControlContainer::OnKeyUpEvent(int dwKey)
{
	// Itera pelos controles filhos encaminhando o evento de key‑up
	SControl* pCurrentControl = static_cast<SControl*>(m_pDown);
	while (pCurrentControl != nullptr)
	{
		// processa somente controles vivos e visíveis
		if (!pCurrentControl->m_cDeleted && pCurrentControl->m_bVisible == 1)
		{
			if (pCurrentControl->OnKeyUpEvent(dwKey) == 1)
			{
				// o controle tratou o evento; interrompe a propagação
				return 1;
			}
		}
		// avança para o próximo controle irmão
		pCurrentControl = static_cast<SControl*>(pCurrentControl->m_pDown);
	}
	return 0;
}
int SControlContainer::OnCharEvent(char iCharCode, int lParam)
{
	return m_pFocusControl == nullptr ? 0 : m_pFocusControl->OnCharEvent(iCharCode, lParam);
}

int SControlContainer::OnChangeIME()
{
	return m_pFocusControl == nullptr ? 0 : m_pFocusControl->OnChangeIME();
}

int SControlContainer::OnIMEEvent(char* ipComposeString)
{
	return m_pFocusControl == nullptr ? 0 : m_pFocusControl->OnIMEEvent(ipComposeString);
}

void SControlContainer::SetFocusedControl(SControl* pControl)
{
	// mantém a regra original do g_nKeyType
	if (g_nKeyType != 1 || (pControl && pControl->m_eCtrlType == CONTROL_TYPE::CTRL_TYPE_EDITABLETEXT))
	{
		if (m_pFocusControl)
			m_pFocusControl->SetFocused(0);

		m_pFocusControl = pControl;

		if (m_pFocusControl)
			m_pFocusControl->SetFocused(1);

		TMScene* pScene = g_pCurrentScene;
		if (pScene)
		{
			const bool bIsEdit =
				(m_pFocusControl != nullptr && m_pFocusControl->m_eCtrlType == CONTROL_TYPE::CTRL_TYPE_EDITABLETEXT);

			if (bIsEdit)
			{
				pScene->m_pAlphaNative->SetVisible(1);
				if (g_pEventTranslator)
					g_pEventTranslator->UpdateCompositionPos();
			}
			else
			{
				pScene->m_pAlphaNative->SetVisible(0);
				pScene->m_pTextIMEDesc->SetVisible(0);
			}

			//  FORÇA SEMPRE ESCONDER os painéis de seleção/lista de chat no FIELD
			if (pScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
			{
				auto* pField = static_cast<TMFieldScene*>(pScene);
				if (pField->m_pChatSelectPanel) pField->m_pChatSelectPanel->SetVisible(0);
				if (pField->m_pChatListPanel)   pField->m_pChatListPanel->SetVisible(0);
			}

			// IME mantém comportamento
			if (m_pFocusControl && m_pFocusControl->IsIMENative())
			{
				if (g_pEventTranslator)
					g_pEventTranslator->SetIMENative();
			}
			else
			{
				if (g_pEventTranslator)
					g_pEventTranslator->SetIMEAlphaNumeric();
			}
		}
	}
}

int SControlContainer::OnControlEvent(DWORD idwControlID, DWORD idwEvent)
{
	return m_pScene ? m_pScene->OnControlEvent(idwControlID, idwEvent) : 0;
}

void SControlContainer::AddItem(SControl* pControl)
{
	m_pControlRoot->AddChild(pControl);
}

int SControlContainer::FrameMove(unsigned int dwServerTime)
{
	TMVector2 vParentPos{};
	auto pCurrentControl = m_pControlRoot;
	auto pRootControl = m_pControlRoot;
	int vControlLayer = 0;
	if (pCurrentControl == nullptr)
		return 1;

	if (m_bInvisibleUI == 1)
		return 1;

	do
	{
		if (!pCurrentControl->m_cDeleted)
		{
			if (pCurrentControl->m_bVisible == 1)
			{
				pCurrentControl->FrameMove2(m_pDrawControl, vParentPos, vControlLayer, 0);

				if (pCurrentControl->m_pDown)
				{
					vParentPos.x += pCurrentControl->m_nPosX;
					vParentPos.y += pCurrentControl->m_nPosY;
					++vControlLayer;

					pCurrentControl = static_cast<SControl*>(pCurrentControl->m_pDown);
					continue;
				}
			}
		}
		else
			m_bCleanUp = 1;

		do
		{
			if (pCurrentControl->m_pNextLink != nullptr)
			{
				pCurrentControl = static_cast<SControl*>(pCurrentControl->m_pNextLink);
				break;
			}

			pCurrentControl = static_cast<SControl*>(pCurrentControl->m_pTop);
			vParentPos.x -= pCurrentControl->m_nPosX;
			vParentPos.y -= pCurrentControl->m_nPosY;
		} while (pCurrentControl != pRootControl && pCurrentControl != nullptr);
	} while (pCurrentControl != pRootControl && pCurrentControl != nullptr);

	if (m_pCursor->m_bVisible)
		m_pCursor->FrameMove2(m_pDrawControl, vParentPos, 29, 0);

	return 1;
}

SControl* SControlContainer::FindControl(unsigned int dwID)
{
	SControl* pCurrentControl = m_pControlRoot;
	SControl* pRootControl = m_pControlRoot;

	if (!pCurrentControl)
		return nullptr;

	do
	{
		if (!pCurrentControl->m_cDeleted)
		{
			if (pCurrentControl->GetControlID() == dwID)
				return pCurrentControl;

			if (pCurrentControl->m_pDown)
			{
				pCurrentControl = static_cast<SControl*>(pCurrentControl->m_pDown);
				continue;
			}
		}

		do
		{
			if (pCurrentControl->m_pNextLink != nullptr)
			{
				pCurrentControl = static_cast<SControl*>(pCurrentControl->m_pNextLink);
				break;
			}

			pCurrentControl = static_cast<SControl*>(pCurrentControl->m_pTop);
		} while (pCurrentControl != pRootControl && pCurrentControl != nullptr);
	} while (pCurrentControl != pRootControl && pCurrentControl != nullptr);

	return nullptr;
}

void SControlContainer::GenerateText(const char* pFileName)
{
	// Skip that function for now... just create a text file with content on screen
	/*FILE* fp = nullptr;
	fopen_s(&fp, pFileName, "wt");

	if (fp)
	{
		SControl* pCurrentControl = m_pControlRoot;
		SControl* pRootControl = m_pControlRoot;

		if (pCurrentControl)
		{
			do 
			{
				if (!pCurrentControl->m_cDeleted)
				{
					if (pCurrentControl->m_eCtrlType == CONTROL_TYPE::CTRL_TYPE_TEXT)
					{
						for (int i = 0 < MAX_RESOURCE_LIST; ++i)
						{
							if (g_pObjectManager->m_ResourceList[i].nNumber == pCurrentControl->m_dwControlID)
							{
								const char* text = TMFont2 pCurrentControl->
							}
						}
					}
				}


			} while (pCurrentControl != pRootControl && pCurrentControl != nullptr);
		}

		fclose(fp);
	}*/
}

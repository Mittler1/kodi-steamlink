/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "GUIDialogPlayEject.h"
#include "guilib/GUIComponent.h"
#include "guilib/GUIWindowManager.h"
#include "storage/MediaManager.h"
#include "utils/log.h"
#include "utils/Variant.h"
#include "utils/XMLUtils.h"
#include "ServiceBroker.h"

#include <utility>

#define ID_BUTTON_PLAY      11
#define ID_BUTTON_EJECT     10

CGUIDialogPlayEject::CGUIDialogPlayEject()
    : CGUIDialogYesNo(WINDOW_DIALOG_PLAY_EJECT)
{
}

CGUIDialogPlayEject::~CGUIDialogPlayEject() = default;

bool CGUIDialogPlayEject::OnMessage(CGUIMessage& message)
{
  if (message.GetMessage() == GUI_MSG_CLICKED)
  {
    int iControl = message.GetSenderId();
    if (iControl == ID_BUTTON_PLAY)
    {
      if (g_mediaManager.IsDiscInDrive())
      {
        m_bConfirmed = true;
        Close();
      }

      return true;
    }
    if (iControl == ID_BUTTON_EJECT)
    {
      g_mediaManager.ToggleTray();
      return true;
    }
  }

  return CGUIDialogYesNo::OnMessage(message);
}

void CGUIDialogPlayEject::FrameMove()
{
  CONTROL_ENABLE_ON_CONDITION(ID_BUTTON_PLAY, g_mediaManager.IsDiscInDrive());

  CGUIDialogYesNo::FrameMove();
}

void CGUIDialogPlayEject::OnInitWindow()
{
  if (g_mediaManager.IsDiscInDrive())
  {
    m_defaultControl = ID_BUTTON_PLAY;
  }
  else
  {
    CONTROL_DISABLE(ID_BUTTON_PLAY);
    m_defaultControl = ID_BUTTON_EJECT;
  }

  CGUIDialogYesNo::OnInitWindow();
}

bool CGUIDialogPlayEject::ShowAndGetInput(const CFileItem & item,
  unsigned int uiAutoCloseTime /* = 0 */)
{
  // Make sure we're actually dealing with a Disc Stub
  if (!item.IsDiscStub())
    return false;

  // Create the dialog
  CGUIDialogPlayEject * pDialog = (CGUIDialogPlayEject *)CServiceBroker::GetGUI()->GetWindowManager().
    GetWindow(WINDOW_DIALOG_PLAY_EJECT);
  if (!pDialog)
    return false;

  // Figure out Lines 1 and 2 of the dialog
  std::string strLine1, strLine2;
  CXBMCTinyXML discStubXML;
  if (discStubXML.LoadFile(item.GetPath()))
  {
    TiXmlElement * pRootElement = discStubXML.RootElement();
    if (!pRootElement || strcmpi(pRootElement->Value(), "discstub") != 0)
      CLog::Log(LOGERROR, "Error loading %s, no <discstub> node", item.GetPath().c_str());
    else
    {
      XMLUtils::GetString(pRootElement, "title", strLine1);
      XMLUtils::GetString(pRootElement, "message", strLine2);
    }
  }

  // Use the label for Line 1 if not defined
  if (strLine1.empty())
    strLine1 = item.GetLabel();

  // Setup dialog parameters
  pDialog->SetHeading(CVariant{219});
  pDialog->SetLine(0, CVariant{429});
  pDialog->SetLine(1, CVariant{std::move(strLine1)});
  pDialog->SetLine(2, CVariant{std::move(strLine2)});
  pDialog->SetChoice(ID_BUTTON_PLAY - 10, 208);
  pDialog->SetChoice(ID_BUTTON_EJECT - 10, 13391);
  if (uiAutoCloseTime)
    pDialog->SetAutoClose(uiAutoCloseTime);

  // Display the dialog
  pDialog->Open();

  return pDialog->IsConfirmed();
}

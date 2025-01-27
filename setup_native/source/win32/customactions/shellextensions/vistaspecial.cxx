/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "shlxtmsi.hxx"

#include <strsafe.h>

#include <systools/win32/uwinapi.h>
#include "../tools/seterror.hxx"

static bool RemoveCompleteDirectoryW(const std::wstring& rPath)
{
    bool bDirectoryRemoved = true;

    std::wstring sPattern = rPath + L"\\" + L"*.*";
    WIN32_FIND_DATAW aFindData;

    // Finding all content in rPath

    HANDLE hFindContent = FindFirstFileW( sPattern.c_str(), &aFindData );

    if ( hFindContent != INVALID_HANDLE_VALUE )
    {
        bool fNextFile = false;
        std::wstring sCurrentDir = L".";
        std::wstring sParentDir = L"..";

        do
        {
            std::wstring sFileName = aFindData.cFileName;

            if (( wcscmp(sFileName.c_str(),sCurrentDir.c_str()) != 0 ) &&
                ( wcscmp(sFileName.c_str(),sParentDir.c_str()) != 0 ))
            {
                std::wstring sCompleteFileName = rPath + L"\\" + sFileName;

                if ( aFindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
                {
                    RemoveCompleteDirectoryW(sCompleteFileName);
                }
                else
                {
                    DeleteFileW( sCompleteFileName.c_str() );
                }
            }

            fNextFile = FindNextFileW( hFindContent, &aFindData );

        } while ( fNextFile );

        FindClose( hFindContent );

        // empty directory can be removed now
        // RemoveDirectory is only successful, if the last handle to the directory is closed
        // -> first removing content -> closing handle -> remove empty directory


        if( !( RemoveDirectoryW(rPath.c_str()) ) )
        {
            bDirectoryRemoved = false;
        }
    }

    return bDirectoryRemoved;
}

/** Move program folder to program_old. Tries 10 times (program_old1, program_old2, ...).
 *
 * @return
 *  ERROR_INSTALL_FAILURE when the folder cannot be moved.
 *  ERROR_SUCCESS otherwise.
 */
extern "C" __declspec(dllexport) UINT __stdcall RenamePrgFolder( MSIHANDLE handle )
{
    std::wstring sOfficeInstallPath = GetMsiPropertyW(handle, L"INSTALLLOCATION");

    std::wstring sRenameSrc = sOfficeInstallPath + L"program";
    std::wstring sRenameDst = sOfficeInstallPath + L"program_old";

    bool bSuccess = MoveFileW( sRenameSrc.c_str(), sRenameDst.c_str() );
    if ( !bSuccess )
    {
        WCHAR sAppend[2] = L"0";
        for ( int i = 0; i < 10; i++ )
        {
            sRenameDst = sOfficeInstallPath + L"program_old" + sAppend;
            bSuccess = MoveFileW( sRenameSrc.c_str(), sRenameDst.c_str() );
            if ( bSuccess )
                break;
            sAppend[0] += 1;
        }
    }

    return bSuccess ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
}


/** Remove leftover program_old folder(s).
 *
 * @return
 *  ERROR_INSTALL_FAILURE when the folder cannot be removed.
 *  ERROR_SUCCESS otherwise.
 */
extern "C" __declspec(dllexport) UINT __stdcall RemovePrgFolder( MSIHANDLE handle )
{
    std::wstring sOfficeInstallPath = GetMsiPropertyW(handle, L"INSTALLLOCATION");
    std::wstring sRemoveDir = sOfficeInstallPath + L"program_old";

    if (!RemoveCompleteDirectoryW(sRemoveDir))
        return ERROR_INSTALL_FAILURE;

    WCHAR sAppend[2] = L"0";
    for ( int i = 0; i < 10; i++ )
    {
        sRemoveDir = sOfficeInstallPath + L"program_old" + sAppend;
        if (!RemoveCompleteDirectoryW( sRemoveDir ))
            return ERROR_INSTALL_FAILURE;
        sAppend[0] += 1;
    }

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

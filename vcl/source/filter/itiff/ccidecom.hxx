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

#pragma once

#include <sal/types.h>
#include <array>
#include <memory>

#define CCI_OPTION_2D               1       // 2D compression (instead of 1D)
#define CCI_OPTION_EOL              2       // There are EOL-Codes at the end of each line.
#define CCI_OPTION_BYTEALIGNEOL     4       // Filling bits before each EOL-Code, so that
                                            // the end of EOL is bytes aligned
#define CCI_OPTION_BYTEALIGNROW     8       // Rows always start byte aligned
#define CCI_OPTION_INVERSEBITORDER  16

// Entry in the Huffman table:
struct CCIHuffmanTableEntry {
    sal_uInt16 nValue;    // The data value.
    sal_uInt16 nCode;     // The code through which the data value is represented.
    sal_uInt16 nCodeBits; // Size of the code in bits.
};

// Entry in a hash table for daft decoding.
struct CCILookUpTableEntry {
    sal_uInt16 nValue;
    sal_uInt16 nCodeBits;
};

class SvStream;

struct DecompressStatus
{
    bool m_bSuccess;
    bool m_bBufferUnchanged;
    DecompressStatus(bool bSuccess, bool bBufferUnchanged)
        : m_bSuccess(bSuccess), m_bBufferUnchanged(bBufferUnchanged)
    {
    }
};

class CCIDecompressor {

public:

    CCIDecompressor( sal_uInt32 nOptions, sal_uInt32 nImageWidth );
    ~CCIDecompressor();

    void StartDecompression( SvStream & rIStream );

    DecompressStatus DecompressScanline(sal_uInt8 * pTarget, sal_uInt64 nTargetBits, bool bLastLine);

private:

    void MakeLookUp(const CCIHuffmanTableEntry * pHufTab,
                    const CCIHuffmanTableEntry * pHufTabSave,
                    CCILookUpTableEntry * pLookUp,
                    sal_uInt16 nHuffmanTableSize,
                    sal_uInt16 nMaxCodeBits);

    bool ReadEOL();

    bool Read2DTag();

    sal_uInt8 ReadBlackOrWhite();

    sal_uInt16 ReadCodeAndDecode(const CCILookUpTableEntry * pLookUp,
                             sal_uInt16 nMaxCodeBits);

    static void FillBits(sal_uInt8 * pTarget, sal_uInt16 nTargetBits,
                  sal_uInt16 nBitPos, sal_uInt16 nNumBits,
                  sal_uInt8 nBlackOrWhite);

    static sal_uInt16 CountBits(const sal_uInt8 * pData, sal_uInt16 nDataSizeBits,
                     sal_uInt16 nBitPos, sal_uInt8 nBlackOrWhite);

    //returns true if pTarget was unmodified
    bool Read1DScanlineData(sal_uInt8 * pTarget, sal_uInt16 nTargetBits);
    bool Read2DScanlineData(sal_uInt8 * pTarget, sal_uInt16 nTargetBits);

    bool m_bTableBad;

    bool m_bStatus;

    std::unique_ptr<sal_uInt8[]> m_pByteSwap;

    SvStream * m_pIStream;

    sal_uInt32 m_nEOLCount;

    sal_uInt32 m_nWidth;

    sal_uInt32 m_nOptions;

    bool m_bFirstEOL;

    std::array<CCILookUpTableEntry, 1<<13> m_pWhiteLookUp;
    std::array<CCILookUpTableEntry, 1<<13> m_pBlackLookUp;
    std::array<CCILookUpTableEntry, 1<<10> m_p2DModeLookUp;
    std::array<CCILookUpTableEntry, 1<<11> m_pUncompLookUp;

    sal_uInt32 m_nInputBitsBuf;
    sal_uInt16 m_nInputBitsBufSize;

    std::unique_ptr<sal_uInt8[]> m_pLastLine;
    sal_uInt64 m_nLastLineSize;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

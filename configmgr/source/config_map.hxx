/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef CONFIG_MAP_HXX
#define CONFIG_MAP_HXX

#include <boost/container/flat_map.hpp>
#include <rtl/ustring.hxx>

// The realisation here is that while a map is a reasonably compact
// representation, there is often no need to have it completely
// sorted, so we can use a fast in-line length comparison as the
// initial compare, rather than sorting of sub string contents.

struct LengthContentsCompare
{
    bool operator()(std::u16string_view a, std::u16string_view b) const
    {
        if (a.size() == b.size())
            return a < b;
        else
            return a.size() < b.size();
    }
};

template <class T>
struct config_map : public boost::container::flat_map<OUString, T, LengthContentsCompare>
{
};

#endif // CONFIG_MAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

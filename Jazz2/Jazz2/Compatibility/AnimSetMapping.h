﻿#pragma once

#include "../../Common.h"
#include "JJ2Version.h"

#include <Containers/StringView.h>

using namespace Death::Containers;

namespace Jazz2::Compatibility
{
    class AnimSetMapping
    {
    public:
        static constexpr char Discard[] = ":discard";

        struct Entry {
            String Category;
            String Name;

            uint8_t Palette[256];
            bool SkipNormalMap;
            int AddBorder;
            bool AllowRealtimePalette;
        };

        AnimSetMapping(JJ2Version version)
            :
            _version(version)
        {
        }

        Entry& Get(int set, int item);

        static AnimSetMapping GetAnimMapping(JJ2Version version);

    private:
        JJ2Version _version;

        void DiscardItems(int advanceBy, JJ2Version appliesTo = JJ2Version::All);
        void SkipItems(int advanceBy = 1);
        void NextSet(int advanceBy = 1, JJ2Version appliesTo = JJ2Version::All);
        void Add(JJ2Version appliesTo, const StringView& category, const StringView& name, uint8_t* palette = nullptr, bool skipNormalMap = false, int addBorder = 0, bool allowRealtimePalette = false);
        void Add(const StringView& category, const StringView& name, uint8_t* palette = nullptr, bool skipNormalMap = false, int addBorder = 0, bool allowRealtimePalette = false);
    };
}
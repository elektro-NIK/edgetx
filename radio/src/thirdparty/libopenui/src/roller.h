/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifndef _ROLLER_H_
#define _ROLLER_H_

#include <touch.h>
#include <choice.h>
#include <font.h>
#include <static.h>
#include "libopenui_globals.h"
#include "libopenui_config.h"

constexpr WindowFlags ROLLER_SEPARATION_LINES = (FORM_FLAGS_LAST << 1u);

class Roller: public Choice {
  public:
    Roller(FormGroup * parent, const rect_t & rect, const char * label, const char * const * values, int16_t vmin, int16_t vmax, std::function<int16_t()> getValue, std::function<void(int16_t)> setValue = nullptr, WindowFlags windowFlags = ROLLER_SEPARATION_LINES):
      Choice(parent, rect, values, vmin, vmax, std::move(getValue), std::move(setValue), windowFlags | NO_SCROLLBAR)
    {
      init(label);
    }

    Roller(FormGroup * parent, const rect_t & rect, const char * label, std::vector<std::string> values, int16_t vmin, int16_t vmax, std::function<int16_t()> getValue, std::function<void(int16_t)> setValue = nullptr, WindowFlags windowFlags = ROLLER_SEPARATION_LINES):
      Choice(parent, rect, std::move(values), vmin, vmax, std::move(getValue), std::move(setValue), windowFlags | NO_SCROLLBAR)
    {
      init(label);
    }

    void init(const char * label)
    {
      if (label) {
        // the label is another window, could be changed, but is it needed?
        new StaticText(parent, {rect.x, rect.y - ROLLER_LINE_HEIGHT, rect.w, ROLLER_LINE_HEIGHT}, label, 0, CENTERED);
      }

      setHeight(ROLLER_LINE_HEIGHT * 3 - 1);
      setPageHeight(ROLLER_LINE_HEIGHT);
      setInnerHeight(INFINITE_HEIGHT);
      updateScrollPositionFromValue();
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "Roller";
    }
#endif

    void paint(BitmapBuffer * dc) override
    {
      int32_t value = getValue();
      auto valuesCount = getValuesCount();

      int index = (scrollPositionY - ROLLER_LINE_HEIGHT + 1)  / ROLLER_LINE_HEIGHT;
      coord_t y = index * ROLLER_LINE_HEIGHT;
      coord_t yMax = scrollPositionY + 3 * ROLLER_LINE_HEIGHT;

      while (y < yMax) {
        auto displayedValue = getValueFromIndex((index + valuesCount) % valuesCount);

        auto fgColor = DISABLE_COLOR;

        if (value == displayedValue) {
          fgColor = FOCUS_COLOR | FONT(STD);
        }

        if ((unsigned)displayedValue < values.size()) {
          dc->drawText(width() / 2, y, values[displayedValue].c_str(), fgColor | CENTERED);
        }
        else {
          dc->drawNumber(width() / 2, y, displayedValue, fgColor | CENTERED);
        }

        if (windowFlags & ROLLER_SEPARATION_LINES) {
          dc->drawSolidHorizontalLine(0, y - 10, width(), DISABLE_COLOR);
        }

        index += 1;
        y += ROLLER_LINE_HEIGHT;
      }
    }

    void checkEvents() override
    {
      Window::checkEvents();

      if (lastEditMode != editMode) {
        updateScrollPositionFromValue();
        lastEditMode = editMode;
      }

#if defined(HARDWARE_TOUCH)
      if (touchState.event != TE_SLIDE) {
        if (scrollPositionY != lastScrollPositionY) {
          updateValueFromScrollPosition();
        }
      }
#endif
    }

  protected:
    coord_t lastScrollPositionY = 0;
    bool lastEditMode = false;

    void updateScrollPositionFromValue()
    {
      setScrollPositionY(ROLLER_LINE_HEIGHT * (getIndexFromValue(this->getValue()) - 1));
      lastScrollPositionY = scrollPositionY;
    }

    void updateValueFromScrollPosition()
    {
      lastScrollPositionY = scrollPositionY;

      auto valuesCount = getValuesCount();
      auto newValue = getValueFromIndex((((scrollPositionY / ROLLER_LINE_HEIGHT) + 1) + valuesCount) % valuesCount);

      setValue(newValue);
      invalidate();
    }
};

#endif // _ROLLER_H_

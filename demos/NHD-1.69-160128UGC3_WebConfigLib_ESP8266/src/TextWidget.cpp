#include <string.h>

#include "TextWidget.h"

#include "NHD.h"
#include "Font.h"


TextWidget::TextWidget()
{
	m_xpos=m_ypos=m_width=m_height=0;

	m_text = 0;
	m_text_backing = 0;
	m_fg_color = WHITE;
	m_bg_color = BLACK;
	m_hmode = HorizontalCenter;
}

void TextWidget::setArea(int x, int y, int width, int height) {
	m_xpos = x;
	m_ypos = y;
	m_width = width;
	m_height = height;
}

char *TextWidget::setTextBackingBuffer(char *buffer) {
	char *prev = m_text_backing;
	m_text_backing = buffer;
	m_text = m_text_backing;
	return prev;
}

void TextWidget::setText(const char *text) {
	if (m_text_backing) {
		strcpy(m_text_backing, text);
	} else {
		if (m_text) free(m_text);
		m_text = strdup(text);
	}
}

void TextWidget::setForegroundColor(const unsigned long fg_color) {
	m_fg_color = fg_color;
}

void TextWidget::setBackgroundColor(const unsigned long bg_color) {
	m_bg_color = bg_color;
}

void TextWidget::setHorizontalMode(const HorizontalMode mode) {
	m_hmode = mode;
}


void TextWidget::render() {
	int textheight = (int)smallFontArrayInfo[0][1];

	// background
	// OLED_FillArea_160128RGB(m_xpos, m_xpos+m_width-1, m_ypos-textheight+1, m_ypos, GREEN);

	if (m_hmode == HorizontalCenter) {
		int x_text = 80 - countPixel(m_text)/2;

		if (x_text > m_xpos) {
			OLED_FillArea_160128RGB(m_xpos, x_text-1, m_ypos-textheight+1, m_ypos, m_bg_color);
		}

		unsigned int x_next = OLED_StringSmallFont_160128RGB(x_text, m_ypos,
				m_text,
				m_fg_color, m_bg_color);

		if (x_next <= m_xpos+m_width-1) {
			// Not everything was covered with text.
			OLED_FillArea_160128RGB(x_next, m_xpos+m_width-1, m_ypos-textheight+1, m_ypos, m_bg_color);
		}
	} else if (m_hmode == HorizontalScroll) {
		// Go row by row and (within that loop) letter by letter
		for (int textrow=0; textrow<textheight; ++textrow) {
			int rowpixels=0;
			OLED_SetPosition_160128RGB(m_xpos, m_ypos-textrow);
			OLED_WriteMemoryStart_160128RGB();
			for (int letter_idx=0; m_text[letter_idx]; ++letter_idx) {
				// Ignore characters being non-ASCII or non-printable:
				if (m_text[letter_idx] < 32) continue;
				if (m_text[letter_idx] >= 127) continue;

				// Draw space between characters.
				if (letter_idx) {
					OLED_Pixel_160128RGB(m_bg_color);
					OLED_Pixel_160128RGB(m_bg_color);
					rowpixels=rowpixels+2;
				}

				int runeIdx = m_text[letter_idx]-32;
				int runeWidth = smallFontArrayInfo[runeIdx][0];
				int runeHeight = smallFontArrayInfo[runeIdx][1];
				int bitlength = smallFontArrayInfo[runeIdx][3];
				runeIdx = smallFontArrayInfo[runeIdx][2]; // reuse

				for (uint8_t byteidx=0; byteidx<runeWidth; ++byteidx) {
					char rowbyte = smallFontArray[runeIdx + byteidx + textrow*runeWidth];
					for (unsigned char mask = 0x80; mask >= 0x01; mask=mask>>1) {
						if ((rowbyte&mask) == mask) {
							OLED_Pixel_160128RGB(m_fg_color);
						} else {
							OLED_Pixel_160128RGB(m_bg_color);
						}
						++rowpixels;
						if (--bitlength < 1) break;
					}
					if (bitlength < 1) break;
				}
				
			}
			// Draw the remaining area.
			for (;rowpixels<m_width;++rowpixels) {
				OLED_Pixel_160128RGB(m_bg_color);
			}
		}
	}
}

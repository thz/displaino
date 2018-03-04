/*
 * Implementation of a text rendering widget.
 *
 * An this provides an abstraction of low-level drawing primitives
 * to achieve higher-level functionality like:
 * 	- alignment of text within an area
 * 	- rendering text of different lengths (also cleaning unused space)
 * 	- scrolling text to allow longer text in small areas
 *
 * 	(c) 2018 Tobias Hintze
 *
 * Use under Apache-License-2.0 conditions is permitted.
 *
 */

#ifndef __TEXTWIDGET_H__
#     define __TEXTWIDGET_H__

class TextWidget {
	public:

		enum HorizontalMode { HorizontalCenter, HorizontalLeft, HorizontalRight, HorizontalScroll };
		TextWidget();
		void setArea(int x, int y, int width, int height);
		void setText(const char *text);
		char *setTextBackingBuffer(char *buffer);
		void setForegroundColor(const unsigned long fg_color);
		void setBackgroundColor(const unsigned long fg_color);
		void setHorizontalMode(const HorizontalMode mode);
		void render();

		int m_xpos;
		int m_ypos;
		int m_width;
		int m_height;

		unsigned long m_fg_color;
		unsigned long m_bg_color;

		HorizontalMode m_hmode;

	protected:
		char *m_text;
		char *m_text_backing;

};

#endif /* __TEXTWIDGET_H__ */

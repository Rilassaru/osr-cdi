
/* ----------------------------------------------------------------------------
Open Source Racing CDI 'OSR-CDI' system for YAMAHA 2T motorcycle
----------------------------------------------------------------------------
Copyright(c) 2013 - 2026, Rilassaru(http://rilassaru.blog.jp/)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met :

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and / or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
----------------------------------------------------------------------------*/

#pragma once
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include "mapdata.h"

#define SELECTED_NONE (-1)
#define MESSAGE_BUFFER_SIZE 256

class MyChart : public Fl_Box
{
private:
	MapData* pData;
	int max_value = 130;
	int ex_interval_w = 5;
	int ex_interval_h = 50;
	int pointer_x = MAX_DATA_NUM / 2;
	double pointer_y = 0;
	int selected_map = SELECTED_NONE;
	int number_of_draw_line = 0;

public:
	Fl_Color map_line_color[4] = { FL_RED,FL_BLUE,FL_DARK_YELLOW ,FL_DARK_GREEN };
	Fl_Color map_bk_color[4] = { FL_RED,FL_BLUE,FL_DARK_YELLOW ,FL_DARK_GREEN };


	MyChart(int X, int Y, int W, int H, const char* l = 0) : Fl_Box(X, Y, W, H, l) {
		pData = NULL;
	}

	int SetLineMapNumber(int num) {
		if (num > MAX_MAP_NUM) {
			num = MAX_MAP_NUM;
		}
		number_of_draw_line = num;
		return num;
	}

	int GetPointerX() {
		return pointer_x;
	}

	double GetMaxValue() {
		return pData->GetMaxValue();
	}

	/**
	 * @ReMap	:Fill in the space before and after the location indicated by index.
	 * @param	:index Location you want to reset
	 * @return	:none
	 * @detail	:
	 */
	void ReMap(int index) {
		int ii, pointer;
		double diff, v1, v2;

		if (SELECTED_NONE == selected_map)
			return;

		pointer = index;

		// before
		if (pointer == ex_interval_w) {
			for (ii = 0; ii < pointer; ii++) {
				pData->Set(selected_map, ii, pData->Get(selected_map, pointer));
			}
		}
		else {
			v1 = pData->Get(selected_map, pointer);
			v2 = pData->Get(selected_map, pointer - ex_interval_w);
			diff = (v1 - v2) / (ex_interval_w);
			for (ii = (pointer - ex_interval_w + 1); ii < pointer; ii++) {
				pData->Set(selected_map, ii, pData->Get(selected_map, ii - 1) + diff);
			}
		}

		// after
		if (pointer >= MAX_DATA_NUM - ex_interval_w) {
			for (ii = pointer; ii < MAX_DATA_NUM; ii++) {
				pData->Set(selected_map, ii, pData->Get(selected_map, pointer));
			}
		}
		else {
			v1 = pData->Get(selected_map, pointer);
			v2 = pData->Get(selected_map, pointer + ex_interval_w);
			diff = (v2 - v1) / (ex_interval_w);
			for (ii = (pointer + 1); ii < pointer + ex_interval_w; ii++) {
				pData->Set(selected_map, ii, pData->Get(selected_map, ii - 1) + diff);
			}
		}
	}

	/**
	 * @SetMapData
	 *			:
	 * @param	: p Set the map data pointer.
	 * @return	:
	 * @detail	:
	 */
	void SetMapData(MapData* p) {
		pData = p;
	}

	/**
	 * @SetMapIndex
	 *			:
	 * @param	: (index) Specify the map to operate.
	 * @return	: Selected map number
	 * @detail	:
	 */
	int SetMapIndex(int index) {
		if (MAX_MAP_NUM > index && index >= 0) {
			selected_map = index;
		}
		else {
			selected_map = SELECTED_NONE;
		}
		return selected_map;
	}

	/**
	 * @MovePointerX
	 *			:
	 * @param	:
	 * @return	:
	 * @detail	:
	 */
	double MovePointerX(int x) {
		x = (x * ex_interval_w) + pointer_x;
		if (x > MAX_DATA_NUM - 1 || x <= 0) {
			return pointer_x;
		}
		pointer_x = x;
		pointer_y = pData->Get(selected_map, pointer_x);
		return pointer_x;
	}

	/**
	 * @MovePointerY
	 *			:
	 * @param	:
	 * @return	:
	 * @detail	:
	 */
	double MovePointerY(double y) {
		double val;

		val = pData->Get(selected_map, pointer_x);

		val += y;
		if (val > max_value) { val = max_value; }
		if (val < 0) { val = 0; }

		pData->Set(selected_map, pointer_x, val);

		pointer_y = val;
		return pointer_y;
	}

	void SetChartSize(int max, int exw=0 , int exh=0) {
		max_value = max;
		if (0 != exw) { ex_interval_w = exw; }	// dash line
		if (0 != exh) { ex_interval_h = exh; }	// dash line
	}

	/**
	 * @draw
	 *			: Reflesh map
	 * @param	: none
	 * @return	: none
	 * @detail	:
	 */
	void draw() {
		int ii, jj, div;
		int x1, x2, y1, y2;

		// pre
		fl_line_style(FL_SOLID, 3);
		div = MAX_DATA_NUM / ex_interval_w;

		fl_push_clip(x(), y(), w(), h());
		fl_font(FL_HELVETICA, 16);

		// back color
		fl_color(FL_WHITE);
		fl_rectf(x(), y(), w(), h());

		// Extension line axis x
		for (ii = 0; ii <= div; ii++) {
			x1 = x() + ((w() * ex_interval_w * ii) / MAX_DATA_NUM);
			x2 = x1 + ((w() * ex_interval_w * (ii + 1)) / MAX_DATA_NUM);

			if ((ii / 5) % 2) {
				fl_color(FL_DARK1);
			}
			else {
				fl_color(FL_WHITE);
			}
			fl_rectf(x1, y(), x2, h());

			fl_color(FL_DARK3);
			fl_line_style(FL_DOT, 2);
			fl_line(x1, y(), x1, y() + h());
		}

		// Extension line axis y
		for (ii = 1; ii <= max_value / ex_interval_h; ii++) {
			y1 = y() + h() - ((h() * ex_interval_h * ii) / max_value);
			fl_color(FL_DARK3);
			fl_line_style(FL_DOT, 2);
			fl_line(x(), y1, x() + w(), y1);
		}

		// Pointer
		
		fl_color(FL_DARK_YELLOW);
		fl_line_style(FL_DOT, 2);
		x1 = (int)(x() + pointer_x * w() / MAX_DATA_NUM);
		y1 = (int)(y() + h() - ((h() * pointer_y) / max_value));
		fl_line(x1, y(), x1, y() + h());

		// Map line
		for (jj = 0; jj < number_of_draw_line; jj++) {
			fl_color(map_line_color[jj]); // MAP0

			if (jj == this->selected_map) {
				fl_line_style(FL_SOLID, 3);
			}
			else {
				fl_line_style(FL_SOLID, 1);
			}

			for (ii = 0; ii < MAX_DATA_NUM - 1; ii++) {
				x1 = x() + ((w() * ii) / MAX_DATA_NUM);
				x2 = x() + ((w() * (ii + 1)) / MAX_DATA_NUM);
				y1 = y() + h() - (int)((h() * pData->Get(jj, ii)) / max_value);
				y2 = y() + h() - (int)((h() * pData->Get(jj, ii + 1)) / max_value);
				fl_line(x1, y1, x2, y2);
			}
		}

		fl_font(FL_HELVETICA, 16);
		fl_draw(label(), x(), y() + h());

		if (SELECTED_NONE != selected_map) {
			char sz[MESSAGE_BUFFER_SIZE];
			//		if (0 == selected_map) { fl_color(FL_RED); }
			//		if (1 == selected_map) { fl_color(FL_BLUE); }
			fl_color(map_line_color[selected_map]);

			sprintf_s(sz, MESSAGE_BUFFER_SIZE, "%d00(rpm):%4.1lf", pointer_x, pData->Get(selected_map, pointer_x));
			fl_font(FL_HELVETICA, 32);
			fl_draw(sz, x() + 10, y() + h() - 32);
		}
		fl_pop_clip();
	}

	~MyChart() {};
};


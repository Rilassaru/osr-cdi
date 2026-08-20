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
#include <FL/Fl_Multiline_Output.H>
#include <stdio.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <FL/fl_draw.H>
//#include <varargs.h>

#define STRING_BUFFER_SIZE_1024	1024

class MyMsg : public Fl_Multiline_Output
{
public:
	MyMsg(int X, int Y, int W, int H, const char* l = 0) : Fl_Multiline_Output(X, Y, W, H, l) {
		if (l) {
			this->label(l);
			this->align(FL_ALIGN_TOP_LEFT);
		}
	};
	void add(const char* str) {
		position(size());
		// wrap and insert long text to fit widget width
		wrap_and_insert(str);
	}

	/**
	 * Insert text with word-wrapping according to widget width.
	 */
	void wrap_and_insert(const char* s) {
		if (!s) return;
		std::string text(s);

		// Determine available width for text inside widget
		int availW = this->w() - 6; // small padding
		if (availW <= 0) { insert(s); return; }

		// Get current last line to measure
		const char* curv = value();
		std::string cur = curv ? std::string(curv) : std::string();
		size_t lastnl = cur.rfind('\n');
		std::string lastline = (lastnl == std::string::npos) ? cur : cur.substr(lastnl + 1);

		int lastw = 0, lh = 0;
		if (!lastline.empty()) fl_measure(lastline.c_str(), lastw, lh);

		// Measure space width
		int spacew = 0; fl_measure(" ", spacew, lh);

		// Process text line by line (respect existing newlines in input)
		std::istringstream lines(text);
		std::string lineStr;
		bool firstLine = true;
		while (std::getline(lines, lineStr)) {
			// For each input line, split into words
			std::istringstream words(lineStr);
			std::string word;
			bool firstWord = true;
			while (words >> word) {
				int ww = 0; fl_measure(word.c_str(), ww, lh);
				if (lastw > 0) {
					// try to append to current line
					if (lastw + spacew + ww <= availW) {
						insert(" ");
						insert(word.c_str());
						lastw += spacew + ww;
					}
					else {
						insert("\n");
						insert(word.c_str());
						lastw = ww;
					}
				}
				else {
					// start new/empty line
					insert(word.c_str());
					lastw = ww;
				}
				firstWord = false;
			}
			// if there are additional (explicit) newlines in input, respect them
			if (!lines.eof()) {
				insert("\n");
				lastw = 0;
			}
		}
	}
	void add(int n) {
		position(size());
		// use std::to_string to avoid sprintf buffer issues
		insert(std::to_string(n).c_str());
	}
	void add(float n) {
		position(size());
		std::ostringstream os;
		os << std::fixed << std::setprecision(6) << n;
		insert(os.str().c_str());
	}
	void add(double n) {
		position(size());
		std::ostringstream os;
		os << std::fixed << std::setprecision(6) << n;
		insert(os.str().c_str());
	}
	void addHex(int n) {
		position(size());
		std::ostringstream os;
		os << std::hex << std::setw(4) << std::setfill('0') << (n & 0xFFFF) << std::dec << " ";
		insert(os.str().c_str());
	}

	void clear() {
		replace(0, size(), "");
	}
};


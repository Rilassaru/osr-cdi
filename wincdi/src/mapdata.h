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
#define MAX_DATA_NUM	160
#define MAX_MAP_NUM		4

class MapData
{
private:

	double map[MAX_MAP_NUM][MAX_DATA_NUM] = {0};
	double min_value = 0, max_value = 0;

public:

	MapData()
	{
	}

	int GetMaxDataNum() { return MAX_DATA_NUM; }
	int GetMaxMapNum()  { return MAX_MAP_NUM;  }

	void SetMinMax(double min, double max) {
		min_value = min;
		max_value = max;
	}

	double Set(int mapno, int index, double value) {
		if (mapno < 0 || mapno >= MAX_MAP_NUM) return -1;
		if (index < 0 || index >= MAX_DATA_NUM) return -1;

		if (value > max_value) { value = max_value; }
		if (value < min_value) { value = min_value; }

		map[mapno][index] = value;
		return value;
	}

	double GetMaxValue() {
		double max_value=0;
		int data_index, map_index;
		for (map_index = 0; map_index < MAX_MAP_NUM; map_index++) {
			for (data_index = 0; data_index < MAX_DATA_NUM; data_index++) {
				if (max_value < map[map_index][data_index]) max_value = map[map_index][data_index];
			}
		}
		return max_value;
	}

	double Get(int mapno, int index) {
		if (mapno < 0 || mapno >= MAX_MAP_NUM) return -1;
		if (index < 0 || index >= MAX_DATA_NUM) return -1;
		return map[mapno][index];
	}

	double* GetPointer() {
		return &map[0][0];
	}

	~MapData()
	{
	}

};


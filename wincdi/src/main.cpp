/* ----------------------------------------------------------------------------
Open Source Racing CDI 'OSR-CDI' system for YAMAHA 2T motorcycle
----------------------------------------------------------------------------
Copyright(c) 2013 - 2026 Rilassaru(http://rilassaru.blog.jp/)
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

#ifdef _DEBUG
#pragma comment( lib, "mt/hidapid.lib" )
#else
#pragma comment( lib, "mt/hidapi.lib" )
#endif

#include <stdint.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_ask.H>

#include "flscreen.h"
#include "HidDevice.h"
#include "mapdata.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <set>
#include <iomanip>

// define
#define WINDOW_TITLE "OSR-CDI v1.5.6"
#define FILE_DESC	"OSR-CDI\t156\n"
#define DEFAULT_FILE_NAME "osr-cdi-autosaved156.cd2"
#define STRING_BUFFER_SIZE 1024

#define MAX_UINT8 0xFF

#define IGN_CHART_MAX	35
#define PVS_CHART_MAX	180

// prototype
void repeat_callback(void*);
void msg_cb(const char* p);
int dataFileWrite(const char* file_name);
int dataFileRead(const char* file_name);
int dataFileReadV14(const char* file_name);
int dataFileReadV15(const char* file_name);

void option_redraw();
void main_window_cb(Fl_Widget*, void*);
void ResizeIgnitionChart();

// global
Fl_Double_Window* w;
HidDevice dev;			// CDI device connector
MapData ignMapData;
MapData pvsMapData;
char szDefaultFileName[STRING_BUFFER_SIZE];
char szCurrentFileName[STRING_BUFFER_SIZE];

/**
 * Main entry point.
 *
 * @param argc Argument count from the runtime environment.
 * @param argv Argument vector from the runtime environment.
 * @return Returns the result of Fl::run() which runs the FLTK event loop.
 */
int main(int argc, char **argv) {
	// -----------------------------------------------------------------
	// Make window
	// -----------------------------------------------------------------
	w = make_window();
	w->callback(main_window_cb);
	w->label(WINDOW_TITLE);


	
	// -----------------------------------------------------------------
	// Setup widgets
	// -----------------------------------------------------------------
	// Init Ignition table & UI
	ignMapData.SetMinMax(0, IGN_CHART_MAX);
	chart_ign->SetMapData(&ignMapData);
	chart_ign->SetChartSize(IGN_CHART_MAX, 5, 5);
	pr_tp_current->minimum(0);
	pr_tp_current->maximum(MAX_UINT8);
	pr_cputemp_current->minimum(-50);
	pr_cputemp_current->maximum(150);

	// Init Power valve table & UI
	pvsMapData.SetMinMax(0, PVS_CHART_MAX);
	chart_pvs->SetMapData(&pvsMapData);
	chart_pvs->SetChartSize(PVS_CHART_MAX, 5, 10);

	// Get default path and make default file name.
	wchar_t wstr[STRING_BUFFER_SIZE];
	size_t returnvalue;
	GetTempPath(STRING_BUFFER_SIZE, wstr);
	wcstombs_s(&returnvalue, szDefaultFileName, STRING_BUFFER_SIZE, wstr, STRING_BUFFER_SIZE);
	strcat_s(szDefaultFileName, STRING_BUFFER_SIZE, DEFAULT_FILE_NAME);

	// read default .cdi
	dataFileRead(szDefaultFileName);

	// Set function pointer to message 
	dev.callback(msg_cb);

	// default active chart is ignition map0
	btn_select_ign_map0->value(1);
	chart_ign->SetMapIndex(0);

	tabs_window->value(tab_device_infomation);
	// Now setup is done.
	w->show(argc, argv);

	// workaround
	w->size(w->w()+1, w->h()+1);

	msg->add("Started successfully.\n");
	msg->add("Waiting device connect.\n");

	Fl::add_timeout(0.1, repeat_callback);

	return(Fl::run());
}

/**
 * Window close callback.
 *
 * Saves the current data to the default file and terminates the process.
 *
 * @param widget The widget that triggered the callback (unused).
 * @param data   User data pointer (unused).
 */
static void main_window_cb(Fl_Widget*, void*) {
	// Save current data and perform a clean shutdown.
	dataFileWrite(szDefaultFileName);

	// Stop periodic callbacks to avoid touching UI after hide
	Fl::remove_timeout(repeat_callback);

	// Close device connection if open
	if (dev.isConnected()) {
		dev.Close();
	}

	// Hide the main window to let Fl::run() return cleanly
	if (w) {
		w->hide();
	}
}

/**
 * Message callback used to display messages in the UI and optionally
 * print them to stdout in debug builds.
 *
 * @param p Null-terminated string message to display.
 */
static void msg_cb(const char* p) {
	msg->add((const char*)p);
#ifdef _DEBUG
	printf("%s", p);
#endif
}


/**
 * Periodic repeat callback executed by FLTK's timeout system.
 *
 * This routine polls HID devices, updates device information fields and
 * schedules itself to run again via Fl::repeat_timeout.
 *
 * @param userdata User-supplied pointer passed by FLTK (unused).
 */
static void repeat_callback(void*) {
	hid_device *handle;
	struct hid_device_info *dev;
	char str[STRING_BUFFER_SIZE] = { 0 };
	usbPacket rxPacket;
	uint8_t txPacket[65] = { 0 };
	size_t returnvalue;

	//return;

	dev = hid_enumerate(DEVICE_VID, DEVICE_PID);
	if (dev) {
		if (dev->manufacturer_string) {
			wcstombs_s(&returnvalue, str, STRING_BUFFER_SIZE, dev->manufacturer_string, STRING_BUFFER_SIZE);
			dev_manufacture->value(str);
		}
		if (dev->product_string) {
			wcstombs_s(&returnvalue, str, STRING_BUFFER_SIZE, dev->product_string, STRING_BUFFER_SIZE);
			dev_product->value(str);
		}
		hid_free_enumeration(dev);
	}
	else {
		dev_manufacture->value("");
		dev_product->value("");
	}


	handle = hid_open(DEVICE_VID, DEVICE_PID, NULL);
	if (handle) {
		txPacket[0] = 0x00;
		txPacket[1] = CMD_GET_STATUS;
		if (hid_write(handle, (unsigned char*)&txPacket, 65) > 0) {
			if (hid_read(handle, (unsigned char*)&rxPacket, 64) > 0) {
				// map sw indicator
				switch (rxPacket.s.current_map) {
				case 0:
					dev_mapsw->color(FL_MAGENTA);
					dev_mapsw->value("MAP1");
					break;
				case 1:
					dev_mapsw->color(FL_CYAN);
					dev_mapsw->value("MAP2");
					break;
				case 2:
					dev_mapsw->color(FL_YELLOW);
					dev_mapsw->value("MAP3");
					break;
				case 3:
					dev_mapsw->color((Fl_Color)87);
					dev_mapsw->value("MAP4");
					break;
				}

				// throttle position indicator
				pr_tp_current->value(rxPacket.s.tp_pot);
				_itoa_s(rxPacket.s.tp_pot, str, STRING_BUFFER_SIZE, 10);
				dev_th_pot->value(str);

				// CPU temperature indicator
				uint16_t cpu_temp = (uint16_t)rxPacket.s.cpu_temp;
				// Convert to voltage (0-5V)
				if (cpu_temp > 0) {
					float v_measured = ((float)cpu_temp * 4.7f) / 1023.0f;
					// Convert voltage to temperature in Celsius
					float temperature_celsius = 25.0f + ((v_measured - 2.2f) / 0.0036f);
					_itoa_s((int)(temperature_celsius), str, STRING_BUFFER_SIZE, 10);
					pr_cputemp_current->value((int)temperature_celsius);
					pr_cputemp_current->copy_label(str);
				}
				else {
					pr_cputemp_current->value(0);
					pr_cputemp_current->copy_label("Not supported");
				}

				// quick shifter indicator
				dev_quickshifter->color(rxPacket.s.qs_signal ? FL_MAGENTA : FL_CYAN);
				dev_quickshifter->value(rxPacket.s.qs_signal ? "QSSW OFF" : "QSSW ON");
				_itoa_s(rxPacket.s.rpm, str, STRING_BUFFER_SIZE, 10);
				dev_rpm->value(str);
				
				// Power valuve
				_itoa_s(rxPacket.s.pv_pot, str, STRING_BUFFER_SIZE, 10);
				dev_pv_pot->value(str);

				_itoa_s(rxPacket.s.pv_target, str, STRING_BUFFER_SIZE, 10);
				dev_pv_target->value(str);

				switch (rxPacket.s.pv_mode) {
				case 0: dev_pv_mode->value("ACTIVE");	break;
				case 1: dev_pv_mode->value("OPEN-1");	break;
				case 2:	dev_pv_mode->value("CLOSE1");	break;
				case 3:	dev_pv_mode->value("OPEN-2");	break;
				case 4:	dev_pv_mode->value("CLOSE-2");	break;
				default: dev_pv_mode->value("UNKOWN");
				}
				_itoa_s(rxPacket.s.pv_mode, str, STRING_BUFFER_SIZE, 10);

				switch (rxPacket.s.pv_mt_state) {
				case 0: dev_pv_mt_state->value("OPEN");		break;
				case 1: dev_pv_mt_state->value("CLOSE");	break;
				case 2:	dev_pv_mt_state->value("STOP");		break;
				case 3:	dev_pv_mt_state->value("BRAKE");	break;
				default: dev_pv_mt_state->value("UNKOWN");
				}
				_itoa_s(rxPacket.s.pv_mode, str, STRING_BUFFER_SIZE, 10);
			}
		}

		dev_devid->value("00");
		dev_revid->value("00");

		hid_close(handle);
	}
	else {
		dev_mapsw->value("");
		dev_quickshifter->value("");
		dev_rpm->value("");
		dev_pv_pot->value("");
		dev_th_pot->value("");
		pr_tp_current->value(0);
		dev_pv_target->value("");
		dev_pv_mode->value("");
		dev_pv_mt_state->value("");
		dev_devid->value("");
		dev_revid->value("");
		pr_cputemp_current->value(-100);
		pr_cputemp_current->copy_label("");
	}
	Fl::repeat_timeout(0.5, repeat_callback);
}

/**
 * Resize the ignition chart scale automatically.
 *
 * Adjusts the ignition chart vertical scale according to the maximum
 * value currently present in the map data so the chart displays data
 * with an appropriate range.
 */
void ResizeIgnitionChart()
{
	// Automatically change the scale according to the maximum value of the map data.
	double max_value = chart_ign->GetMaxValue();
	int new_scale;
	if (max_value >= IGN_CHART_MAX) {
		new_scale = ((int)((max_value + 5) / 5) * 5);
		ignMapData.SetMinMax(0, new_scale);
	}
	else {
		new_scale = IGN_CHART_MAX;
	}
	chart_ign->SetChartSize(new_scale, 5, 5);

}

/**
 * Increase map value handler.
 *
 * Called by the UI when the up/raise control is activated. Moves the
 * selected pointer upward in the currently active map (ignition or PVS).
 *
 * @param btn The repeat button widget (unused).
 * @param data User data pointer (unused).
 */
void cb_up(Fl_Repeat_Button*, void*) {
	// Take the movement magnification
	int move = (radio_map_x1->value() * 1) + (radio_map_x5->value() * 5) + (radio_map_x10->value() * 10);

	// For the ignition timing map, the amount of movement is 0.1,
	// and for the power valve, the amount of movement is 1.

	// Processing when the ignition timing map is currently selected.
	if (btn_select_ign_map0->value() || btn_select_ign_map1->value() || btn_select_ign_map2->value() || btn_select_ign_map3->value()) {
		// Refresh
		chart_ign->MovePointerY(move*0.1);
		chart_ign->ReMap(chart_ign->GetPointerX());
		ResizeIgnitionChart();
		chart_ign->redraw();
	}

	// Processing when the YPVS map is currently selected.
	if (btn_select_pvs_map0->value() || btn_select_pvs_map1->value() || btn_select_pvs_map2->value() || btn_select_pvs_map3->value()) {
		chart_pvs->MovePointerY(move);
		chart_pvs->ReMap(chart_ign->GetPointerX());
		chart_pvs->redraw();
	}
}

/**
 * Decrease map value handler.
 *
 * Called by the UI when the down/decrease control is activated. Moves the
 * selected pointer downward in the currently active map (ignition or PVS).
 *
 * @param btn The repeat button widget (unused).
 * @param data User data pointer (unused).
 */
void cb_down(Fl_Repeat_Button*, void*){
	// Take the movement magnification
	int move = (radio_map_x1->value() * 1) + (radio_map_x5->value() * 5) + (radio_map_x10->value() * 10);

	// For the ignition timing map, the amount of movement is 0.1,
	// and for the power valve, the amount of movement is 1.

	// Processing when the ignition timing map is currently selected.
	if (btn_select_ign_map0->value() || btn_select_ign_map1->value() || btn_select_ign_map2->value() || btn_select_ign_map3->value()) {
		// Refresh
		chart_ign->MovePointerY(move*-0.1);
		chart_ign->ReMap(chart_ign->GetPointerX());
		ResizeIgnitionChart();
		chart_ign->redraw();
	}

	// Processing when the YPVS map is currently selected.
	if (btn_select_pvs_map0->value() || btn_select_pvs_map1->value() || btn_select_pvs_map2->value() || btn_select_pvs_map3->value()) {
		chart_pvs->MovePointerY(move*-1);
		chart_pvs->ReMap(chart_ign->GetPointerX());
		chart_pvs->redraw();
	}
}

/**
 * Move pointer left handler.
 *
 * Moves the selected pointer to the left in the active map when the user
 * requests a left shift.
 *
 * @param btn The repeat button widget (unused).
 * @param data User data pointer (unused).
 */
void cb_left(Fl_Repeat_Button*, void*){
	chart_ign->MovePointerX(-1);
	chart_ign->redraw();
	chart_pvs->MovePointerX(-1);
	chart_pvs->redraw();
}

/**
 * Move pointer right handler.
 *
 * Moves the selected pointer to the right in the active map when the user
 * requests a right shift.
 *
 * @param btn The repeat button widget (unused).
 * @param data User data pointer (unused).
 */
void cb_right(Fl_Repeat_Button*, void*){
	chart_ign->MovePointerX(1);
	chart_ign->redraw();
	chart_pvs->MovePointerX(1);
	chart_pvs->redraw();
}
/**
 * Open file button handler.
 *
 * Presents a file chooser to the user and attempts to load the selected
 * settings file into the application.
 *
 * @param btn The button widget that triggered the action (unused).
 * @param data User data pointer (unused).
 */
void cb_file_open(Fl_Button*, void*) {
	char str[STRING_BUFFER_SIZE] = { 0 };
	Fl_Native_File_Chooser fnfc;
	fnfc.options(Fl_Native_File_Chooser::NO_OPTIONS);
	fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
	fnfc.title("Load CDI data");
	//fnfc.filter("CDI data\t*.cd2");
	fnfc.filter("CDI data\t*.{cd2,cdi}");
	int ret = fnfc.show();

	if (0 == ret) {

		// 2pass file read
		if (dataFileRead(fnfc.filename())&& dataFileRead(fnfc.filename())) {
			ResizeIgnitionChart();
			chart_ign->redraw();
			chart_pvs->redraw();
			option_redraw();

			// rename window title
			strcpy_s(str, STRING_BUFFER_SIZE, WINDOW_TITLE);
			strcat_s(str, STRING_BUFFER_SIZE, " - ");
			strcat_s(str, STRING_BUFFER_SIZE, fnfc.filename());
			strcpy_s(szCurrentFileName, STRING_BUFFER_SIZE, fnfc.filename());
			w->label(str);
			return;
		}
	}
	else if (ret < 0) {
		fl_beep(FL_BEEP_ERROR);
		fl_alert("Cannot open file or write data.");
	}
	w->flush();
}

/**
 * Save file button handler.
 *
 * Saves the current configuration to the specified file. If no filename is
 * provided this will prompt or use the default auto-save location.
 *
 * @param btn The button widget that triggered the action (unused).
 * @param data User data pointer (unused).
 */
void cb_file_save(Fl_Button*, void*) {
	char str[STRING_BUFFER_SIZE] = { 0 };
	Fl_Native_File_Chooser fnfc;
	fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	fnfc.title("Save CDI data");
	fnfc.filter("CDI data\t*.cd2");
	fnfc.preset_file(szCurrentFileName);
	fnfc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM | Fl_Native_File_Chooser::NEW_FOLDER);
	int ret = fnfc.show();

	if (0 == ret) {
		if (dataFileWrite(fnfc.filename())) {
			// rename window title
			strcpy_s(str, STRING_BUFFER_SIZE, WINDOW_TITLE);
			strcat_s(str, STRING_BUFFER_SIZE, " - ");
			strcat_s(str, STRING_BUFFER_SIZE, fnfc.filename());
			strcpy_s(szCurrentFileName, STRING_BUFFER_SIZE, fnfc.filename());

			w->label(str);
			return;
		}
	}
	else if (ret < 0) {
		fl_beep(FL_BEEP_ERROR);
		fl_alert("Cannot open file or write data.");
	}
}

// ----------------------------------------------------------------------------
// Device read button down
// ----------------------------------------------------------------------------
/**
 * Read configuration from the connected device.
 *
 * Initiates a read operation from the connected CDI device and updates the
 * UI with received values.
 *
 * @param btn The button widget that triggered the action (unused).
 * @param data User data pointer (unused).
 */
void cb_device_read(Fl_Button*, void*) {
	char str[STRING_BUFFER_SIZE] = { 0 };

	if (false == dev.isConnected()) {
		fl_beep(FL_BEEP_ERROR);
		fl_alert("USB CDI not found.");
		return;
	}

	uint8_t current_engine_type;

	current_engine_type = dev.cfg.sys_pulse_per_rotation;

	if (FALSE == dev.readTable(ignMapData.GetPointer(), pvsMapData.GetPointer()))
	{
		fl_alert("Reading device map error.");
	}
	else {
		fl_alert("Reading device map successfully.");
		strcpy_s(str, STRING_BUFFER_SIZE, WINDOW_TITLE);
		strcat_s(str, STRING_BUFFER_SIZE, " - device loaded");
		w->label(str);

	}

	// Engine type check
	if (current_engine_type != dev.cfg.sys_pulse_per_rotation) {
		fl_beep(FL_BEEP_ERROR);
		fl_alert("WARNING: Engine type does not match!\nCheck your config.");
		radio_single_pulse->value(1==dev.cfg.sys_pulse_per_rotation);
		radio_twin_pulse->value(2 == dev.cfg.sys_pulse_per_rotation);
	}

	if (0 == dev.cfg.sys_pickup_degree) {
		fl_beep(FL_BEEP_ERROR);
		fl_alert("WARNING: Pickup degree is zero!\nCheck your config.");
		dev.cfg.sys_pickup_degree = (1 == dev.cfg.sys_pulse_per_rotation ? PICKUP_DEG_1 : PICKUP_DEG_2);
	}

	// chart draw
	ResizeIgnitionChart();
	chart_ign->redraw();
	chart_pvs->redraw();
	option_redraw();

	w->flush();
}

/**
 * Write configuration to the connected device.
 *
 * Sends the current configuration values to the connected CDI device.
 *
 * @param btn The button widget that triggered the action (unused).
 * @param data User data pointer (unused).
 */
void cb_device_write(Fl_Button*, void*) {
	msg->clear();

	if (false==dev.isConnected()) {
		fl_beep(FL_BEEP_ERROR);
		fl_alert("USB CDI not found.");
		return;
	}

	// Writing phase.
	fl_beep(FL_BEEP_QUESTION);
	if (0 == fl_ask("Do you really want to write data to CDI?")) {
		return;
	}

	if (FALSE == dev.writeTable(ignMapData.GetPointer(), pvsMapData.GetPointer())) {
		fl_alert("Writing device map error(ph.1).");
		chart_ign->redraw();
		chart_pvs->redraw();
		option_redraw();
		dev.Close();
		return;
	}

	dev.Close();

	msg->redraw();
}

/**
 * Write configuration to a .cd2 file.
 *
 * Writes the current maps and device configuration to disk. If the
 * provided filename does not end with ".cd2" the extension will be
 * appended automatically.
 *
 * @param file_name Path of the file to write.
 * @return 1 on success, 0 on failure.
 */
int dataFileWrite(const char* file_name) {
	std::string fname(file_name ? file_name : "");
	if (fname.size() == 0) return 0;
	// Ensure extension .cd2
	if (fname.size() <= 4 || fname.substr(fname.size() - 4) != ".cd2") {
		fname += ".cd2";
	}

	std::ofstream ofs(fname, std::ios::out | std::ios::trunc);
	if (!ofs.is_open()) return 0;

	ofs << FILE_DESC;
	if (!ofs) return 0;

	// map data
	for (int map = 0; map < MAX_MAP_NUM; ++map) {
		for (int index = 0; index < MAX_DATA_NUM; ++index) {
			ofs << "IG:" << map << '\t' << std::setw(3) << std::setfill('0') << index
				<< '\t' << std::fixed << std::setprecision(6) << ignMapData.Get(map, index) << '\n';
			if (!ofs) return 0;
		}
	}
	for (int map = 0; map < MAX_MAP_NUM; ++map) {
		for (int index = 0; index < MAX_DATA_NUM; ++index) {
			ofs << "PV:" << map << '\t' << std::setw(3) << std::setfill('0') << index
				<< '\t' << std::fixed << std::setprecision(6) << pvsMapData.Get(map, index) << '\n';
			if (!ofs) return 0;
		}
	}

	// Config entries (pad numbers to 3 digits to match previous format)
	auto writeKey = [&](const char* key, int value) {
		ofs << key << '\t' << std::setw(3) << std::setfill('0') << value << '\n';
	};

	writeKey("QS-RPM", dev.cfg.qs_enable_rpm);
	writeKey("QS_ONT", dev.cfg.qs_sw_on_count);
	writeKey("QS-CUT", dev.cfg.qs_cut_count);
	writeKey("QS-DIS", dev.cfg.qs_disable_count);
	writeKey("rev_limit", dev.cfg.rev_limit);
	writeKey("PulseType", dev.cfg.sys_pulse_per_rotation);
	writeKey("PickupDeg", dev.cfg.sys_pickup_degree);
	writeKey("QSOPT-UseAsStopSW", dev.cfg.qs_use_as_stopsw);
	writeKey("QSOPT-SwitchReverse", dev.cfg.qs_reverse_onoff_state);
	writeKey("2nd_wave_ignition_count", dev.cfg.count_of_an_ignition);
	writeKey("OPT_C2port_usage", dev.cfg.sys_opt_port);
	writeKey("TP_TYPE", dev.cfg.tp_type);
	writeKey("TP_TH01", dev.cfg.tp_threshold01);
	writeKey("TP_TH02", dev.cfg.tp_threshold02);
	writeKey("TP_TH03", dev.cfg.tp_threshold03);
	writeKey("rpm_limit_for_an_ignition", dev.cfg.sys_rpm_for_an_ignition);
	writeKey("pvs_init_pattern", dev.cfg.sys_pvs_init_pattern);

	ofs.close();
	return ofs.fail() ? 0 : 1;
}

// ----------------------------------------------------------------------------
// Entry point for file reading
// ----------------------------------------------------------------------------
int dataFileRead(const char* file_name) {
	// Clear previous messages and log start
	msg->clear();
	std::string fname = file_name ? file_name : "";
	msg->add((std::string("Loading file: ") + fname + "\n").c_str());

	// Determine file extension and dispatch to the appropriate reader
	int len = strlen(file_name);
	if (len > 4 && 0 == _stricmp(&file_name[len - 4], ".cdi")) {
		// Legacy Ver1.4 format
		msg->add("Detected format: Ver1.4\n");
		int ret = dataFileReadV14(file_name);
		if (ret) msg->add("File load completed.\n");
		else msg->add("File load failed.\n");
		return ret;
	}
	else {
		// Current Ver1.5 format
		msg->add("Detected format: Ver1.5\n");
		int ret = dataFileReadV15(file_name);
		if (ret) msg->add("File load completed.\n");
		else msg->add("File load failed.\n");
		return ret;
	}
}

/**
 * Read .CDI file in Ver1.5 format.
 *
 * Parses the Ver1.5 formatted .cd2 file and populates map and
 * configuration data structures.
 *
 * @param file_name Path of the file to read.
 * @return 1 on success, 0 on failure.
 */
// Generic parser for .cd2/.cdi files. Populates maps and device config.
static bool ParseCdiFile(const char* file_name, int &outErrorFlag) {
	std::ifstream ifs(file_name);
	if (!ifs.is_open()) {
		msg->add("Error @ Can't open file\n");
		return false;
	}

	outErrorFlag = 0;
	std::string line;
	int lineNo = 0;
	int errorCount = 0;
	int igEntries = 0;
	int pvEntries = 0;
	std::set<int> igMaps;
	std::set<int> pvMaps;
	std::set<std::string> unknownKeys;
	std::set<std::string> presentKeys;
	while (std::getline(ifs, line)) {
		++lineNo;
		// Trim CR
		if (!line.empty() && line.back() == '\r') line.pop_back();
		// Skip empty lines
		if (line.empty()) continue;

		// Split by tab first
		std::vector<std::string> parts;
		size_t start = 0;
		while (true) {
			size_t pos = line.find('\t', start);
			if (pos == std::string::npos) {
				parts.push_back(line.substr(start));
				break;
			}
			parts.push_back(line.substr(start, pos - start));
			start = pos + 1;
		}

		// If first token contains header marker, skip
		if (!parts.empty()) {
			if (parts[0].find("OSR-CDI") != std::string::npos) continue;
			// IG: lines
			if (parts[0].rfind("IG:", 0) == 0 && parts.size() >= 3) {
				try {
					int map = std::stoi(parts[0].substr(3));
					int index = std::stoi(parts[1]);
					double value = std::stod(parts[2]);
					if (map >= 0 && map < MAX_MAP_NUM && index >= 0 && index < MAX_DATA_NUM) {
						ignMapData.Set(map, index, value);
						++igEntries;
						igMaps.insert(map);
					}
					else {
						std::ostringstream os; os << "Parse error line " << lineNo << ": IG index out of range: '" << line << "'";
						msg->add(os.str().c_str());
						++errorCount;
					}
				}
				catch (const std::exception &ex) {
					std::ostringstream os; os << "Parse error line " << lineNo << ": IG parse failed: '" << line << "' (" << ex.what() << ")";
					msg->add(os.str().c_str());
					outErrorFlag = 1;
				}
				catch (...) {
					std::ostringstream os; os << "Parse error line " << lineNo << ": IG parse failed: '" << line << "'";
					msg->add(os.str().c_str());
					outErrorFlag = 1;
				}
				continue;
			}

			// PV: lines
			if (parts[0].rfind("PV:", 0) == 0 && parts.size() >= 3) {
				try {
					int map = std::stoi(parts[0].substr(3));
					int index = std::stoi(parts[1]);
					double value = std::stod(parts[2]);
					if (map >= 0 && map < MAX_MAP_NUM && index >= 0 && index < MAX_DATA_NUM) {
						pvsMapData.Set(map, index, value);
						++pvEntries;
						pvMaps.insert(map);
					}
					else {
						std::ostringstream os; os << "Parse error line " << lineNo << ": PV index out of range: '" << line << "'";
						msg->add(os.str().c_str());
						++errorCount;
					}
				}
				catch (const std::exception &ex) {
					std::ostringstream os; os << "Parse error line " << lineNo << ": PV parse failed: '" << line << "' (" << ex.what() << ")";
					msg->add(os.str().c_str());
					++errorCount;
				}
				catch (...) {
					std::ostringstream os; os << "Parse error line " << lineNo << ": PV parse failed: '" << line << "'";
					msg->add(os.str().c_str());
					++errorCount;
				}
				continue;
			}

			// Key-value config lines: key\tvalue
			if (parts.size() >= 2) {
				std::string key = parts[0];
				std::string val = parts[1];
				// trim spaces
				auto trim = [](std::string &s) {
					while (!s.empty() && std::isspace((unsigned char)s.front())) s.erase(s.begin());
					while (!s.empty() && std::isspace((unsigned char)s.back())) s.pop_back();
				};
				trim(key); trim(val);

				// case-insensitive key comparison: normalize to upper-case
				std::string keyU = key;
				std::transform(keyU.begin(), keyU.end(), keyU.begin(), [](unsigned char c){ return (char)std::toupper(c); });

				int n = 0;
				try { n = std::stoi(val); }
				catch (const std::exception &ex) {
					std::ostringstream os; os << "Parse error line " << lineNo << ": invalid numeric value for '" << key << "': '" << val << "' (" << ex.what() << ")";
					msg->add(os.str().c_str());
					++errorCount; continue;
				}
				catch (...) {
					std::ostringstream os; os << "Parse error line " << lineNo << ": invalid numeric value for '" << key << "': '" << val << "'";
					msg->add(os.str().c_str());
					++errorCount; continue;
				}

				if (keyU == "QS-RPM") dev.cfg.qs_enable_rpm = (uint8_t)n;
				else if (keyU == "QS_ONT") dev.cfg.qs_sw_on_count = (uint8_t)n;
				else if (keyU == "QS-CUT") dev.cfg.qs_cut_count = (uint8_t)n;
				else if (keyU == "QS-DIS") dev.cfg.qs_disable_count = (uint8_t)n;
				else if (keyU == "REV_LIMIT") dev.cfg.rev_limit = (uint8_t)n;
				else if (keyU == "PULSETYPE") dev.cfg.sys_pulse_per_rotation = (uint8_t)n;
				else if (keyU == "PICKUPDEG" || keyU == "PICKUP_DEG") dev.cfg.sys_pickup_degree = (uint8_t)n;
				else if (keyU == "QSOPT-USEASSTOPSW" ) dev.cfg.qs_use_as_stopsw = (uint8_t)n;
				else if (keyU == "OPT-STP") dev.cfg.qs_use_as_stopsw = (uint8_t)n; // legacy key mapping
				else if (keyU == "QSOPT-SWITCHREVERSE") dev.cfg.qs_reverse_onoff_state = (uint8_t)n;
				else if (keyU == "OPT-REV") dev.cfg.qs_reverse_onoff_state = (uint8_t)n; // legacy key mapping
				else if (keyU == "2ND_WAVE_IGNITION_COUNT") dev.cfg.count_of_an_ignition = (uint8_t)n;
				else if (keyU == "OPT_C2PORT_USAGE") dev.cfg.sys_opt_port = (uint8_t)n;
				else if (keyU == "TP_TYPE") dev.cfg.tp_type = (uint8_t)n;
				else if (keyU == "TP_TH01") dev.cfg.tp_threshold01 = (uint8_t)n;
				else if (keyU == "TP_TH02") dev.cfg.tp_threshold02 = (uint8_t)n;
				else if (keyU == "TP_TH03") dev.cfg.tp_threshold03 = (uint8_t)n;
				else if (keyU == "RPM_LIMIT_FOR_AN_IGNITION") dev.cfg.sys_rpm_for_an_ignition = (uint8_t)n;
				else if (keyU == "PVS_INIT_PATTERN") dev.cfg.sys_pvs_init_pattern = (uint8_t)n;
				else {
					// Unknown key: collect and log as info (non-fatal)
					unknownKeys.insert(key);
					std::ostringstream os; os << "Info line " << lineNo << ": unknown key '" << key << "' - ignored";
					msg->add(os.str().c_str());
				}
				// record that this key was present
				presentKeys.insert(keyU);
			}
		}
	}

	// Fill defaults for missing option keys (regression safety)
	// List of keys to ensure defaults: TP_TYPE, TP_TH01/02/03, PVS_INIT_PATTERN, OPT_C2PORT_USAGE
	auto ensureDefault = [&](const std::string &k, int defaultVal, int &target) {
		if (presentKeys.find(k) == presentKeys.end()) {
			target = (int)defaultVal;
			std::ostringstream os; os << "Default applied: " << k << "=" << defaultVal << "\n";
			msg->add(os.str().c_str());
		}
	};

	ensureDefault("TP_TYPE", 0, (int&)dev.cfg.tp_type);
	ensureDefault("TP_TH01", 64, (int&)dev.cfg.tp_threshold01);
	ensureDefault("TP_TH02", 128, (int&)dev.cfg.tp_threshold02);
	ensureDefault("TP_TH03", 192, (int&)dev.cfg.tp_threshold03);
	ensureDefault("PVS_INIT_PATTERN", 0, (int&)dev.cfg.sys_pvs_init_pattern);
	ensureDefault("OPT_C2PORT_USAGE", 0, (int&)dev.cfg.sys_opt_port);

	// Summary
	std::ostringstream sum;
	sum << "Parsed IG entries: " << igEntries << ", distinct IG maps: " << igMaps.size() << "\n";
	sum << "Parsed PV entries: " << pvEntries << ", distinct PV maps: " << pvMaps.size() << "\n";
	msg->add(sum.str().c_str());

	if (!unknownKeys.empty()) {
		std::ostringstream uk;
		uk << "Unknown keys: ";
		bool first = true;
		for (const auto &k : unknownKeys) {
			if (!first) uk << ", "; first = false;
			uk << k;
		}
		uk << "\n";
		msg->add(uk.str().c_str());
	}

	if (errorCount > 0) {
		std::ostringstream ec; ec << "Parse errors: " << errorCount << "\n";
		msg->add(ec.str().c_str());
	}

	outErrorFlag = (errorCount > 0) ? 1 : 0;
	return true;
}

int dataFileReadV15(const char* file_name) {
	int errorFlag = 0;
	if (!ParseCdiFile(file_name, errorFlag)) return 0;

	// Adjust derived fields
	if (2 == dev.cfg.sys_pulse_per_rotation) {
		dev.cfg.sys_pickup_degree = PICKUP_DEG_2;
		dev.count_of_tune = COUNT_OF_TUNE_2;
		dev.cfg.sys_numerator_for_rpm = SYS_NUMERATOR_FOR_RPM_2;
	} else {
		dev.cfg.sys_pickup_degree = PICKUP_DEG_1;
		dev.count_of_tune = COUNT_OF_TUNE_1;
		dev.cfg.sys_numerator_for_rpm = SYS_NUMERATOR_FOR_RPM_1;
	}

	ResizeIgnitionChart();
	if (errorFlag) {
		chart_ign->redraw();
		chart_pvs->redraw();
		option_redraw();
		fl_alert("File load error.\n Check cdi file and all settings carefully.");
		w->flush();
		return 0;
	}

	chart_ign->redraw();
	chart_pvs->redraw();
	option_redraw();
	w->flush();
	return 1;
}

// ----------------------------------------------------------------------------
// .CDI File reading (Ver1.4 format)
// ----------------------------------------------------------------------------
/**
 * Read .CDI file in Ver1.4 format.
 *
 * Legacy reader for older .cd2 files that follow the Ver1.4 layout. This
 * function populates the map data and configuration fields accordingly.
 *
 * @param file_name Path of the file to read.
 * @return 1 on success, 0 on failure.
 */
int dataFileReadV14(const char* file_name) {
	int errorFlag = 0;
	if (!ParseCdiFile(file_name, errorFlag)) return 0;

	// Copy map1-2 to map3-4 to match legacy behavior
	msg->add("Copy map1-2 to map3-4\n");
	for (int index = 0; index < MAX_DATA_NUM; index++) {
		ignMapData.Set(2, index, ignMapData.Get(0, index)); // Map1 -> Map3
		ignMapData.Set(3, index, ignMapData.Get(1, index)); // Map2 -> Map4
		pvsMapData.Set(2, index, pvsMapData.Get(0, index));
		pvsMapData.Set(3, index, pvsMapData.Get(1, index));
	}

	ResizeIgnitionChart();

	if (errorFlag) {
		chart_ign->redraw();
		chart_pvs->redraw();
		option_redraw();
		fl_alert("File load error.\n Check cdi file and all settings carefully.");
		w->flush();
		return 0;
	}

	chart_ign->redraw();
	chart_pvs->redraw();
	option_redraw();
	w->flush();
	return 1;
}

/**
 * Update option screen UI from device configuration.
 *
 * Copies values from the device configuration into the option UI
 * controls so the user sees the current device state.
 */
void option_redraw() {
	char str[STRING_BUFFER_SIZE];

	vs_qs_enable_rpm->value((double)dev.cfg.qs_enable_rpm);
	vs_qs_sw_on_count->value((double)dev.cfg.qs_sw_on_count);
	vs_qs_cut_count->value((double)dev.cfg.qs_cut_count);
	vs_qs_disable_count->value((double)dev.cfg.qs_disable_count);
	vs_rev_limit->value((double)dev.cfg.rev_limit);

	if (1 == dev.cfg.sys_pulse_per_rotation) {
		radio_single_pulse->value(1);
		radio_twin_pulse->value(0);
	}
	else {
		radio_single_pulse->value(0);
		radio_twin_pulse->value(1);
	}

	_itoa_s(dev.cfg.sys_pickup_degree, str, STRING_BUFFER_SIZE, 10);
	sys_pickup_degree->value(str);

	btn_qs_reverse_onoff_state->value(dev.cfg.qs_reverse_onoff_state);
	btn_qs_use_as_stopsw->value(dev.cfg.qs_use_as_stopsw);

	vs_count_of_an_ignition->value((double)dev.cfg.count_of_an_ignition);
	vs_rpm_for_an_ignition->value((double)dev.cfg.sys_rpm_for_an_ignition);

	if (0 == dev.cfg.sys_opt_port) {
		radio_optc2_pulse_out->value(1);
		radio_optc2_map_out->value(0);
	}
	else {
		radio_optc2_pulse_out->value(0);
		radio_optc2_map_out->value(1);
	}

	if (0 == dev.cfg.sys_pvs_init_pattern) {

		radio_opt_pv_init_open_end->value(1);
		radio_opt_pv_init_close_end->value(0);
	}
	else {
		radio_opt_pv_init_open_end->value(0);
		radio_opt_pv_init_close_end->value(1);
	}

	switch (dev.cfg.tp_type) {
	case 0:
		chart_ign->SetLineMapNumber(1);
		chart_pvs->SetLineMapNumber(1);

		radio_opt_mapsw_type1->value(1);
		radio_opt_mapsw_type2->value(0);
		radio_opt_mapsw_type4->value(0);

		btn_select_ign_map1->deactivate();
		btn_select_ign_map2->deactivate();
		btn_select_ign_map3->deactivate();
		btn_select_pvs_map1->deactivate();
		btn_select_pvs_map2->deactivate();
		btn_select_pvs_map3->deactivate();

		break;
	case 1:
		chart_ign->SetLineMapNumber(2);
		chart_pvs->SetLineMapNumber(2);

		radio_opt_mapsw_type1->value(0);
		radio_opt_mapsw_type2->value(1);
		radio_opt_mapsw_type4->value(0);

		btn_select_ign_map1->activate();
		btn_select_ign_map2->deactivate();
		btn_select_ign_map3->deactivate();
		btn_select_pvs_map1->activate();
		btn_select_pvs_map2->deactivate();
		btn_select_pvs_map3->deactivate();
		break;
	case 2:
		chart_ign->SetLineMapNumber(4);
		chart_pvs->SetLineMapNumber(4);

		radio_opt_mapsw_type1->value(0);
		radio_opt_mapsw_type2->value(0);
		radio_opt_mapsw_type4->value(1);

		btn_select_ign_map1->activate();
		btn_select_ign_map2->activate();
		btn_select_ign_map3->activate();
		btn_select_pvs_map1->activate();
		btn_select_pvs_map2->activate();
		btn_select_pvs_map3->activate();
		break;

	}

	vs_throttle_1to2->value(dev.cfg.tp_threshold01);
	vs_throttle_2to3->value(dev.cfg.tp_threshold02);
	vs_throttle_3to4->value(dev.cfg.tp_threshold03);

	if (radio_opt_mapsw_type4->value()) {
		vs_throttle_1to2->activate();
		vs_throttle_2to3->activate();
		vs_throttle_3to4->activate();
	}
	else {
		vs_throttle_1to2->deactivate();
		vs_throttle_2to3->deactivate();
		vs_throttle_3to4->deactivate();
	}

}

/**
 * Handler for option sliders change.
 *
 * Updates device configuration fields when option sliders change.
 *
 * @param slider The slider widget that changed (unused).
 * @param data User data pointer (unused).
 */
void cb_option_setting_vs(Fl_Value_Slider*, void*) {
	dev.cfg.qs_enable_rpm = (uint8_t)vs_qs_enable_rpm->value();
	dev.cfg.qs_sw_on_count = (uint8_t)vs_qs_sw_on_count->value();
	dev.cfg.qs_cut_count = (uint8_t)vs_qs_cut_count->value();
	dev.cfg.qs_disable_count = (uint8_t)vs_qs_disable_count->value();
	dev.cfg.rev_limit = (uint8_t)vs_rev_limit->value();
	dev.cfg.count_of_an_ignition = (uint8_t)vs_count_of_an_ignition->value();
	dev.cfg.sys_rpm_for_an_ignition = (uint8_t)vs_rpm_for_an_ignition->value();
}

/**
 * Handler for option button (toggle) changes.
 *
 * Updates boolean configuration flags when option buttons change.
 *
 * @param btn The light button widget that changed (unused).
 * @param data User data pointer (unused).
 */
void cb_option_setting_btn(Fl_Light_Button*, void*) {
	dev.cfg.qs_reverse_onoff_state = (uint8_t)(btn_qs_reverse_onoff_state->value() ? 1 : 0);
	dev.cfg.qs_use_as_stopsw = (uint8_t)(btn_qs_use_as_stopsw->value() ? 1 : 0);
}

/**
 * Engine type selection changed callback.
 *
 * Adjusts internal configuration values based on the selected engine
 * pulse type (single or twin).
 *
 * @param btn The round button widget that triggered the change (unused).
 * @param data User data pointer (unused).
 */
void cb_engine_type(Fl_Round_Button*, void*) {
	dev.cfg.sys_pulse_per_rotation = radio_single_pulse->value() + (radio_twin_pulse->value() * 2);

	if (2 != dev.cfg.sys_pulse_per_rotation && 1 != dev.cfg.sys_pulse_per_rotation) {
		dev.cfg.sys_pulse_per_rotation = 2;  // default twin pulse type
	}

	if (2 == dev.cfg.sys_pulse_per_rotation) {
		dev.cfg.sys_pickup_degree = PICKUP_DEG_2;
		dev.count_of_tune = COUNT_OF_TUNE_2;
		dev.cfg.sys_numerator_for_rpm = SYS_NUMERATOR_FOR_RPM_2;
	}
	else {
		dev.cfg.sys_pickup_degree = PICKUP_DEG_1;
		dev.count_of_tune = COUNT_OF_TUNE_1;
		dev.cfg.sys_numerator_for_rpm = SYS_NUMERATOR_FOR_RPM_1;
	}
	option_redraw();
}

/**
 * Pickup degree text input changed callback.
 *
 * Parses the pickup degree value entered by the user and stores it in
 * device configuration.
 *
 * @param input The integer input widget (unused).
 * @param data User data pointer (unused).
 */
void cb_sys_pickup_degree(Fl_Int_Input*, void*) {
	dev.cfg.sys_pickup_degree = atoi(sys_pickup_degree->value());
}

/**
 * Map selection callback.
 *
 * Sets the active map index for ignition and PVS charts based on the
 * selected UI radio buttons and requests a redraw.
 *
 * @param btn The light button that triggered the selection (unused).
 * @param data User data pointer (unused).
 */
void cb_select_map(Fl_Light_Button*, void*) {
	chart_ign->SetMapIndex(SELECTED_NONE);
	chart_pvs->SetMapIndex(SELECTED_NONE);

	if (btn_select_ign_map0->value()) { chart_ign->SetMapIndex(0); }
	if (btn_select_ign_map1->value()) { chart_ign->SetMapIndex(1); }
	if (btn_select_ign_map2->value()) { chart_ign->SetMapIndex(2); }
	if (btn_select_ign_map3->value()) { chart_ign->SetMapIndex(3); }

	if (btn_select_pvs_map0->value()) { chart_pvs->SetMapIndex(0); }
	if (btn_select_pvs_map1->value()) { chart_pvs->SetMapIndex(1); }
	if (btn_select_pvs_map2->value()) { chart_pvs->SetMapIndex(2); }
	if (btn_select_pvs_map3->value()) { chart_pvs->SetMapIndex(3); }

	chart_ign->redraw();
	chart_pvs->redraw();
}

/**
 * Option port type changed callback.
 *
 * Updates device configuration to reflect the selected option port
 * behaviour (pulse out or map out).
 *
 * @param btn The round button widget that triggered the change (unused).
 * @param data User data pointer (unused).
 */
void cb_optport_type(Fl_Round_Button*, void*) {
	if (radio_optc2_pulse_out->value()) { dev.cfg.sys_opt_port = 0; }
	else if (radio_optc2_map_out->value())	{ dev.cfg.sys_opt_port = 1; }
}

/**
 * Power valve initial pattern type changed callback.
 *
 * Updates the device configuration for the initial PVS pattern based
 * on the selected radio button.
 *
 * @param btn The round button widget that triggered the change (unused).
 * @param data User data pointer (unused).
 */
void cb_pviinit_type(Fl_Round_Button*, void*) {
	if (radio_opt_pv_init_open_end->value()) { dev.cfg.sys_pvs_init_pattern = 0; }
	else if (radio_opt_pv_init_close_end->value()) { dev.cfg.sys_pvs_init_pattern = 1; }
}

/**
 * Map switch type changed callback.
 *
 * Adjusts throttle-based map switching mode and resets map selection to
 * map 0 after a change.
 *
 * @param btn The round button widget that triggered the change (unused).
 * @param data User data pointer (unused).
 */
void cb_opt_mapsw_type(Fl_Round_Button*, void*) {
	if (radio_opt_mapsw_type1->value()) { dev.cfg.tp_type = 0; }
	if (radio_opt_mapsw_type2->value()) { dev.cfg.tp_type = 1; }
	if (radio_opt_mapsw_type4->value()) { dev.cfg.tp_type = 2; }

	btn_select_ign_map0->value(1);
	chart_ign->SetMapIndex(0);
	chart_pvs->SetMapIndex(SELECTED_NONE);
	option_redraw();
}

/**
 * Throttle threshold changed handler.
 *
 * Validates and normalizes throttle threshold slider values and stores
 * them into device configuration.
 *
 * @param slider The value slider widget that changed (unused).
 * @param data User data pointer (unused).
 */
void cb_opt_setting_throttle(Fl_Value_Slider*, void*) {
	double th1, th2, th3;
	th1 = vs_throttle_1to2->value();
	th2 = vs_throttle_2to3->value();
	th3 = vs_throttle_3to4->value();
	
	if (th3 < 3) { th3 = 3; }
	if (th2 < 2) { th2 = 2; }
	if (th1 >= th2) { th1 = th2 - 1; }
	if (th2 >= th3) { th2 = th3 - 1; }

	vs_throttle_1to2->value(th1);
	vs_throttle_2to3->value(th2);
	vs_throttle_3to4->value(th3);

	dev.cfg.tp_threshold01 = (uint8_t)th1;
	dev.cfg.tp_threshold02 = (uint8_t)th2;
	dev.cfg.tp_threshold03 = (uint8_t)th3;
}


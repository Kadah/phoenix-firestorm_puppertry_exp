/**
 * @file llpresetsmanager.h
 * @brief Implementation for the LLPresetsManager class.
 *
 * $LicenseInfo:firstyear=2007&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#ifndef LL_PRESETSMANAGER_H
#define LL_PRESETSMANAGER_H

#include "llcombobox.h"

#include <list>
#include <map>

static const std::string PRESETS_DEFAULT = "Default";
static const std::string PRESETS_DEFAULT_UPPER = "DEFAULT";
static const std::string PRESETS_DIR = "presets";
static const std::string PRESETS_GRAPHIC = "graphic";
static const std::string PRESETS_CAMERA = "camera";

// Files in presets/camera/ folder will be the first word in the names plus '.xml'.  i.e. 'Rear.xml'
static const std::string PRESETS_REAR_VIEW = "Rear View";
static const std::string PRESETS_FRONT_VIEW = "Front View";
static const std::string PRESETS_SIDE_VIEW = "Side View";
static const std::string PRESETS_CLOSE_UP = "Close-Up";

enum EDefaultOptions
{
	DEFAULT_SHOW,
	DEFAULT_TOP,
	DEFAULT_BOTTOM,
	DEFAULT_HIDE				// Do not display "Default" in a list
};

class LLPresetsManager : public LLSingleton<LLPresetsManager>
{
	LLSINGLETON(LLPresetsManager);
	~LLPresetsManager();

public:

	typedef std::list<std::string> preset_name_list_t;
	typedef boost::signals2::signal<void()> preset_list_signal_t;

	void createMissingDefault(const std::string& subdirectory);
	void startWatching(const std::string& subdirectory);
	void triggerChangeCameraSignal();
	void triggerChangeSignal();
	static std::string getPresetsDir(const std::string& subdirectory);
	bool setPresetNamesInComboBox(const std::string& subdirectory, LLComboBox* combo, EDefaultOptions default_option);
	void loadPresetNamesFromDir(const std::string& subdirectory, preset_name_list_t& presets, EDefaultOptions default_option);
	bool savePreset(const std::string& subdirectory, std::string name, bool createDefault = false);
	void loadPreset(const std::string& subdirectory, std::string name);
	bool deletePreset(const std::string& subdirectory, std::string name);
	bool isCameraDirty();
	static void setCameraDirty(bool dirty);

	void createCameraDefaultPresets();

	bool isTemplateCameraPreset(std::string preset_name);
	bool isDefaultCameraPreset(std::string preset_name);
	void resetCameraPreset(std::string preset_name);
	bool createDefaultCameraPreset(std::string preset_name, bool force_reset = false);

	// Emitted when a preset gets loaded, deleted, or saved.
	boost::signals2::connection setPresetListChangeCameraCallback(const preset_list_signal_t::slot_type& cb);
	boost::signals2::connection setPresetListChangeCallback(const preset_list_signal_t::slot_type& cb);

	// Emitted when a preset gets loaded or saved.

	preset_name_list_t mPresetNames;

	preset_list_signal_t mPresetListChangeCameraSignal;
	preset_list_signal_t mPresetListChangeSignal;

	// <FS:Ansariel> Graphic preset controls independent from XUI
	void setIsLoadingPreset(bool is_loading) { mIsLoadingPreset = is_loading; }
	void setIsDrawDistanceSteppingActive(bool is_active) { mIsDrawDistanceSteppingActive = is_active; }
	// </FS:Ansariel>

  private:
	LOG_CLASS(LLPresetsManager);

	void getControlNames(std::vector<std::string>& names);
	static void settingChanged();

	boost::signals2::connection	mCameraChangedSignal;

	static bool	mCameraDirty;
	static bool mIgnoreChangedSignal;

	// <FS:Ansariel> Graphic preset controls independent from XUI
	void initGraphicPresetControlNames();
	void initGraphicPresetControls();
	void handleGraphicPresetControlChanged(LLControlVariablePtr control, const LLSD& new_value, const LLSD& old_value);
	bool mIsLoadingPreset;
	bool mIsDrawDistanceSteppingActive;
	std::vector<std::string> mGraphicPresetControls;
	// </FS:Ansariel>
};

#endif // LL_PRESETSMANAGER_H

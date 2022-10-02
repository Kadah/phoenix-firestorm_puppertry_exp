/**
 * @file llpanellandaudio.cpp
 * @brief Allows configuration of "media" for a land parcel,
 *   for example movies, web pages, and audio.
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

#include "llviewerprecompiledheaders.h"

#include "llpanellandaudio.h"

// viewer includes
#include "llmimetypes.h"
#include "llviewerparcelmgr.h"
#include "llviewerregion.h"
#include "lluictrlfactory.h"

// library includes
#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "llfloaterurlentry.h"
#include "llfocusmgr.h"
//#include "lllineeditor.h"	// <FS:CR> FIRE-593 - Unused since we use a combobox instead
#include "llparcel.h"
#include "lltextbox.h"
#include "llradiogroup.h"
#include "llspinctrl.h"
#include "llsdutil.h"
#include "lltexturectrl.h"
#include "roles_constants.h"
#include "llscrolllistctrl.h"

// Firestorm includes
#include "llviewercontrol.h"	// <FS:CR> FIRE-593 - Needed for gSavedSettings where we store our media list
#include "llclipboard.h"

// Values for the parcel voice settings radio group
enum
{
	kRadioVoiceChatEstate = 0,
	kRadioVoiceChatPrivate = 1,
	kRadioVoiceChatDisable = 2
};

//---------------------------------------------------------------------------
// LLPanelLandAudio
//---------------------------------------------------------------------------

LLPanelLandAudio::LLPanelLandAudio(LLParcelSelectionHandle& parcel)
:	LLPanel(/*std::string("land_media_panel")*/), mParcel(parcel)
{
}


// virtual
LLPanelLandAudio::~LLPanelLandAudio()
{
}


BOOL LLPanelLandAudio::postBuild()
{
	mCheckSoundLocal = getChild<LLCheckBoxCtrl>("check sound local");
	childSetCommitCallback("check sound local", onCommitAny, this);

	mCheckParcelEnableVoice = getChild<LLCheckBoxCtrl>("parcel_enable_voice_channel");
	childSetCommitCallback("parcel_enable_voice_channel", onCommitAny, this);

	// This one is always disabled so no need for a commit callback
	mCheckEstateDisabledVoice = getChild<LLCheckBoxCtrl>("parcel_enable_voice_channel_is_estate_disabled");

	mCheckParcelVoiceLocal = getChild<LLCheckBoxCtrl>("parcel_enable_voice_channel_local");
	childSetCommitCallback("parcel_enable_voice_channel_local", onCommitAny, this);

// <FS:CR> FIRE-593 - We use a combobox now, not a line editor, also set callbacks for new add/remove stream buttons
	//mMusicURLEdit = getChild<LLLineEditor>("music_url");
	mMusicURLEdit = getChild<LLComboBox>("music_url");
	childSetCommitCallback("music_url", onCommitAny, this);
	
	mBtnStreamAdd = getChild<LLButton>("stream_add_btn");
	mBtnStreamAdd->setCommitCallback(boost::bind(&LLPanelLandAudio::onBtnStreamAdd, this));
	
	mBtnStreamDelete = getChild<LLButton>("stream_delete_btn");
	mBtnStreamDelete->setCommitCallback(boost::bind(&LLPanelLandAudio::onBtnStreamDelete, this));
	
	mBtnStreamCopyToClipboard = getChild<LLButton>("stream_copy_btn");
	mBtnStreamCopyToClipboard->setCommitCallback(boost::bind(&LLPanelLandAudio::onBtnCopyToClipboard, this));
// </FS:CR>

	mCheckAVSoundAny = getChild<LLCheckBoxCtrl>("all av sound check");
	childSetCommitCallback("all av sound check", onCommitAny, this);

	mCheckAVSoundGroup = getChild<LLCheckBoxCtrl>("group av sound check");
	childSetCommitCallback("group av sound check", onCommitAny, this);

    mCheckObscureMOAP = getChild<LLCheckBoxCtrl>("obscure_moap");
    childSetCommitCallback("obscure_moap", onCommitAny, this);

	return TRUE;
}


// public
void LLPanelLandAudio::refresh()
{
	LLParcel *parcel = mParcel->getParcel();

	if (!parcel)
	{
		clearCtrls();
	}
	else
	{
		// something selected, hooray!

		// Display options
		BOOL can_change_media = LLViewerParcelMgr::isParcelModifiableByAgent(parcel, GP_LAND_CHANGE_MEDIA);

		mCheckSoundLocal->set( parcel->getSoundLocal() );
		mCheckSoundLocal->setEnabled( can_change_media );

		bool allow_voice = parcel->getParcelFlagAllowVoice();

		LLViewerRegion* region = LLViewerParcelMgr::getInstance()->getSelectionRegion();
		if (region && region->isVoiceEnabled())
		{
			mCheckEstateDisabledVoice->setVisible(false);

			mCheckParcelEnableVoice->setVisible(true);
			mCheckParcelEnableVoice->setEnabled( can_change_media );
			mCheckParcelEnableVoice->set(allow_voice);

			mCheckParcelVoiceLocal->setEnabled( can_change_media && allow_voice );
		}
		else
		{
			// Voice disabled at estate level, overrides parcel settings
			// Replace the parcel voice checkbox with a disabled one
			// labelled with an explanatory message
			mCheckEstateDisabledVoice->setVisible(true);

			mCheckParcelEnableVoice->setVisible(false);
			mCheckParcelEnableVoice->setEnabled(false);
			mCheckParcelVoiceLocal->setEnabled(false);
		}

		mCheckParcelEnableVoice->set(allow_voice);
		mCheckParcelVoiceLocal->set(!parcel->getParcelFlagUseEstateVoiceChannel());

// <FS:CR> FIRE-593 - Populate the audio combobox with our saved urls, then add the parcel's current url up top.
		//mMusicURLEdit->setText(parcel->getMusicURL());
		std::string current_url = parcel->getMusicURL();
		mMusicURLEdit->clearRows();
		LLSD streamlist = gSavedSettings.getLLSD("FSStreamList");
		LLSD& streams = streamlist["audio"];

		for (LLSD::array_iterator s_itr = streams.beginArray(); s_itr != streams.endArray(); ++s_itr)
		{
			mMusicURLEdit->add(LLSD(*s_itr));
			LL_DEBUGS() << "adding: " << *s_itr << " to the audio stream combo." << LL_ENDL;
		}
		mMusicURLEdit->addSeparator(ADD_TOP);
		mMusicURLEdit->add(LLSD(current_url), ADD_TOP);
		mMusicURLEdit->selectByValue(current_url);
		
		mBtnStreamAdd->setEnabled( can_change_media );
		mBtnStreamDelete->setEnabled( can_change_media );
		mBtnStreamCopyToClipboard->setEnabled(TRUE);
// </FS:CR>
		mMusicURLEdit->setEnabled( can_change_media );

		// <FS:Testy> FIRE-29157 - Remove invalid URLs that were rejected by the server
		if (current_url != mLastSetURL)
		{
			mMusicURLEdit->remove(mLastSetURL);
			LLSD::Integer index = 0;
			for (LLSD::array_iterator iter = streams.beginArray(), end = streams.endArray(); iter != end; ++iter, ++index)
			{
				if ((*iter).asString() == mLastSetURL)
				{
					LL_WARNS() << "Removing stream from saved streams list because it was rejected by the region: " << mLastSetURL << LL_ENDL;
					streams.erase(index);
					break;
				}
			}
			gSavedSettings.setLLSD("FSStreamList", streamlist);
		}
		// </FS:Testy>

		BOOL can_change_av_sounds = LLViewerParcelMgr::isParcelModifiableByAgent(parcel, GP_LAND_OPTIONS) && parcel->getHaveNewParcelLimitData();
		mCheckAVSoundAny->set(parcel->getAllowAnyAVSounds());
		mCheckAVSoundAny->setEnabled(can_change_av_sounds);

		mCheckAVSoundGroup->set(parcel->getAllowGroupAVSounds() || parcel->getAllowAnyAVSounds());	// On if "Everyone" is on
		mCheckAVSoundGroup->setEnabled(can_change_av_sounds && !parcel->getAllowAnyAVSounds());		// Enabled if "Everyone" is off

        mCheckObscureMOAP->set(parcel->getObscureMOAP());
        mCheckObscureMOAP->setEnabled(can_change_media);
	}
}
// static
void LLPanelLandAudio::onCommitAny(LLUICtrl*, void *userdata)
{
	LLPanelLandAudio *self = (LLPanelLandAudio *)userdata;

	LLParcel* parcel = self->mParcel->getParcel();
	if (!parcel)
	{
		return;
	}

	// Extract data from UI
	BOOL sound_local		= self->mCheckSoundLocal->get();
// <FS:CR> FIRE-593 - It's a combobox now
	//std::string music_url = self->mMusicURLEdit->getText();
	std::string music_url = self->mMusicURLEdit->getSimple();
// </FS:CR>

	BOOL voice_enabled = self->mCheckParcelEnableVoice->get();
	BOOL voice_estate_chan = !self->mCheckParcelVoiceLocal->get();

	BOOL any_av_sound		= self->mCheckAVSoundAny->get();
	BOOL group_av_sound		= TRUE;		// If set to "Everyone" then group is checked as well
	if (!any_av_sound)
	{	// If "Everyone" is off, use the value from the checkbox
		group_av_sound = self->mCheckAVSoundGroup->get();
	}

    bool obscure_moap = self->mCheckObscureMOAP->get();

	// Remove leading/trailing whitespace (common when copying/pasting)
	LLStringUtil::trim(music_url);

	// <FS> Add leading http:// if not already present
	if (!music_url.empty() && music_url.find("://") == std::string::npos)
	{
		music_url.insert(0, "http://");
	}
	// </FS>

	// <FS:Testy> FIRE-29157 - Remove invalid URLs that were rejected by the server
	self->mLastSetURL = music_url;
	// </FS:Testy>

	// Push data into current parcel
	parcel->setParcelFlag(PF_ALLOW_VOICE_CHAT, voice_enabled);
	parcel->setParcelFlag(PF_USE_ESTATE_VOICE_CHAN, voice_estate_chan);
	parcel->setParcelFlag(PF_SOUND_LOCAL, sound_local);
	parcel->setMusicURL(music_url);
	parcel->setAllowAnyAVSounds(any_av_sound);
	parcel->setAllowGroupAVSounds(group_av_sound);
    parcel->setObscureMOAP(obscure_moap);

	// Send current parcel data upstream to server
	LLViewerParcelMgr::getInstance()->sendParcelPropertiesUpdate( parcel );

	// Might have changed properties, so let's redraw!
	self->refresh();
}

// <FS:CR> FIRE-593 - Add/remove streams from the list with these. They're fantastic!
void LLPanelLandAudio::onBtnStreamAdd()
{
	std::string music_url = mMusicURLEdit->getSimple();
	LLStringUtil::trim(music_url);
	
	if (!music_url.empty())
	{
		LLSD streamlist = gSavedSettings.getLLSD("FSStreamList");

		bool has_url = false;
		for (LLSD::array_const_iterator it = streamlist["audio"].beginArray(); it != streamlist["audio"].endArray(); ++it)
		{
			if ((*it).asString() == music_url)
			{
				has_url = true;
				break;
			}
		}

		if (!has_url)
		{
			streamlist["version"] = 1;
			streamlist["audio"].append(music_url);
			gSavedSettings.setLLSD("FSStreamList", streamlist);
			refresh();
		}
		else
		{
			LL_INFOS() << "Could not add stream to saved streams list because it is already in the list: " << music_url << LL_ENDL;
		}
	}
}

void LLPanelLandAudio::onBtnStreamDelete()
{
	std::string music_url = mMusicURLEdit->getSimple();
	LLStringUtil::trim(music_url);
	std::string music_url_no_http;
	if (music_url.find("http://") == 0)
	{
		music_url_no_http = music_url.substr(7, music_url.size() - 7);
	}

	LLSD streamlist = gSavedSettings.getLLSD("FSStreamList");
	LLSD streamlist_new;
	streamlist_new["version"] = 1;

	for (LLSD::array_const_iterator it = streamlist["audio"].beginArray(); it != streamlist["audio"].endArray(); ++it)
	{
		std::string current_url = (*it).asString();
		if (current_url != music_url && !(current_url.find("://") == std::string::npos && current_url == music_url_no_http))
		{
			streamlist_new["audio"].append(current_url);
		}
	}

	if (streamlist["audio"].size() == streamlist_new["audio"].size())
	{
		LL_WARNS() << "Could not remove stream from saved streams list: " << music_url << LL_ENDL;
	}

	gSavedSettings.setLLSD("FSStreamList", streamlist_new);
	refresh();
}

void LLPanelLandAudio::onBtnCopyToClipboard()
{
	std::string music_url = mMusicURLEdit->getSimple();
	LLStringUtil::trim(music_url);
	
	if (!music_url.empty())
	{
		LLClipboard::instance().copyToClipboard(utf8str_to_wstring(music_url), 0, music_url.size() );
	}
}
// </FS:CR>

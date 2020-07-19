#pragma once

#include "wx/wx.h"
#include <thread>
#include <future>

#ifndef __BASE_H
#define __BASE_H

class cMain : public wxFrame
{
public:
	cMain();
	~cMain();

	bool done;

	wxPanel* panel;
	wxButton* buttonOpenFile;
	wxButton* buttonRun;
	wxChoice* compressDecompressChoice;
	wxChoice* fileTypeChoice;
	wxTextCtrl* fileNameInput;

	wxStaticText* currentFileLabel;
	wxStaticText* statusLabel;

	void OnButtonOpenFilePress(wxCommandEvent& event);
	void OnButtonRunPress(wxCommandEvent& event);
	void OnCompressDecopressChoice(wxCommandEvent& event);
	void OnFileTypeChoice(wxCommandEvent& event);

	void doWork(wxString docPath, wxString fileName, std::promise<float>&& compressionRate);
	wxString generateWorkMessage(int i);

	wxString docPath;
	wxString fileName;
	std::promise<float> compressionRatePromise;

	int compressDecopmressSelected = 0;
	int fileTypeSelected = 0;

	DECLARE_EVENT_TABLE()
};

enum
{
	BUTTON_OpenFile = wxID_HIGHEST + 1,
	BUTTON_Run,
	CHOICE_CompressDecompress,
	CHOICE_FileType
};

#endif
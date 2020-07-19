#include "cMain.h"
#include "cApp.h"
#include "ShannonFano.h"
#include "sun.xpm"

BEGIN_EVENT_TABLE(cMain, wxFrame)
EVT_BUTTON(BUTTON_OpenFile, cMain::OnButtonOpenFilePress)
EVT_BUTTON(BUTTON_Run, cMain::OnButtonRunPress)
EVT_CHOICE(CHOICE_CompressDecompress, cMain::OnCompressDecopressChoice)
EVT_CHOICE(CHOICE_FileType, cMain::OnFileTypeChoice)
END_EVENT_TABLE()

using namespace std::literals::chrono_literals;

cMain::cMain() : wxFrame(nullptr, wxID_ANY, "Shannon Fano Compress", wxDefaultPosition, wxSize(400, 240), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX))
{
	SetIcon(sun);
	Centre();

	const wxString compressDecompressArr[2] = { "Compress", "Decompress" };
	const wxString fileTypeArr[3] = { "TXT", "BMP", };

	panel = new wxPanel(this, wxID_ANY);

    buttonOpenFile = new wxButton(panel, BUTTON_OpenFile, "Open file", wxPoint(10, 10), wxSize(90, 30));
	currentFileLabel = new wxStaticText(panel, wxID_ANY, "Opened file:", wxPoint(110, 17), wxDefaultSize);

	new wxStaticText(panel, wxID_ANY, "Mode:", wxPoint(12, 73), wxDefaultSize);
	compressDecompressChoice = new wxChoice(panel, CHOICE_CompressDecompress, wxPoint(110, 70), wxSize(90, 30), 2, compressDecompressArr);
	compressDecompressChoice->SetSelection(0);

	new wxStaticText(panel, wxID_ANY, "File type:", wxPoint(12, 103), wxDefaultSize);
	fileTypeChoice = new wxChoice(panel, CHOICE_FileType, wxPoint(110, 100), wxSize(90, 30), 2, fileTypeArr);
	fileTypeChoice->SetSelection(0);
	fileTypeChoice->Enable(false);

	new wxStaticText(panel, wxID_ANY, "File name:", wxPoint(12, 133), wxDefaultSize);
	fileNameInput = new wxTextCtrl(panel, CHOICE_FileType, "output", wxPoint(110, 130), wxSize(90, 23));

	buttonRun = new wxButton(panel, BUTTON_Run, "Run", wxPoint(250, 97), wxSize(90, 30));

	statusLabel = new wxStaticText(panel, wxID_ANY, "Select a file and run!", wxPoint(145, 175), wxSize(200, 15));
}

cMain::~cMain()
{
}

void cMain::OnButtonOpenFilePress(wxCommandEvent& event)
{
	wxFileDialog* OpenDialog;
	if (compressDecopmressSelected == 0)
	{
		OpenDialog = new wxFileDialog(panel, _("Choose a file to open"), wxEmptyString, wxEmptyString, "Text files (*.txt)|*.txt|Bitmap files (*.bmp)|*.bmp", wxFD_OPEN, wxDefaultPosition);
	}
	else
	{
		OpenDialog = new wxFileDialog(panel, _("Choose a file to open"), wxEmptyString, wxEmptyString, "Binary file (*.bin)|*.bin", wxFD_OPEN, wxDefaultPosition);
	}
	
	if (OpenDialog->ShowModal() == wxID_OK)
	{
		docPath = OpenDialog->GetPath();
		currentFileLabel->SetLabel(wxString("Opened file: ") << OpenDialog->GetFilename());
	}

	OpenDialog->Destroy();
}

void cMain::OnButtonRunPress(wxCommandEvent& event)
{
	if (docPath == "")
	{
		statusLabel->SetLabel("Open a file first!");
		statusLabel->SetPosition(wxPoint(145, 175));
		return;
	}

	if (fileNameInput->GetValue() == "")
	{
		statusLabel->SetLabel("Type in a file name!");
		statusLabel->SetPosition(wxPoint(145, 175));
		return;
	}

	wxString fileType;
	switch (fileTypeSelected)
	{
		case 0:
			fileType = ".txt";
			break;
		case 1:
			fileType = ".bmp";
			break;
		default:
			fileType = ".txt";
	}

	if (compressDecopmressSelected == 0) 
	{
		fileName = fileNameInput->GetValue() + ".bin";

		done = false;

		compressionRatePromise = std::promise<float>();
		std::thread worker([this] { this->doWork(docPath, fileName, std::move(compressionRatePromise)); });

		statusLabel->SetPosition(wxPoint(175, 175));
		
		unsigned int i = 0;
		while(!done)
		{
			statusLabel->SetLabel(generateWorkMessage(i));
			std::this_thread::sleep_for(0.1s);
			i++;
			wxYield();
		}

		worker.join();
		float compressionRate = compressionRatePromise.get_future().get();

		statusLabel->SetLabel("Done! Compression rate: " + std::to_string(compressionRate) + "%");
		statusLabel->SetPosition(wxPoint(110, 175));
	}
	else
	{
		wxString fileName = fileNameInput->GetValue() + fileType;
		ShannonFano::decompressor(docPath, fileName);
		statusLabel->SetLabel("Done!");
		statusLabel->SetPosition(wxPoint(175, 175));
	}
}

void cMain::OnCompressDecopressChoice(wxCommandEvent& event)
{
	compressDecopmressSelected = compressDecompressChoice->GetSelection();

	if (compressDecopmressSelected == 0) fileTypeChoice->Enable(false);
	else if (compressDecopmressSelected == 1) fileTypeChoice->Enable(true);

	docPath = "";
	currentFileLabel->SetLabel("Open file:");
}

void cMain::OnFileTypeChoice(wxCommandEvent& event)
{
	fileTypeSelected = fileTypeChoice->GetSelection();
}

void cMain::doWork(wxString docPath, wxString fileName, std::promise<float>&& compressionRate)
{
	float compressionRateRan = ShannonFano::compressor(docPath, fileName);
	compressionRate.set_value(compressionRateRan);

	done = true;	
}

wxString cMain::generateWorkMessage(int i)
{
	switch (i % 3)
	{
	case 0:
		return "Working.";
	case 1:
		return "Working..";
	case 2:
		return "Working...";
	default:
		return "Working.";
	}
}
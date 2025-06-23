#include <Windows.h>
#include <cstdint>

enum class PentaneTarget {
	Invalid = 0,
	CarsTheVideoGame = 1,
	CarsMaterNationalChampionship = 2,
	Cars2TheVideoGame = 3,
	Cars2TheVideoGameArcade = 4,
	Cars3DrivenToWin = 5,
};

auto get_target_from_timestamp(DWORD timestamp) -> PentaneTarget {
	switch (timestamp) {
	case 0x445A4258:
		return PentaneTarget::CarsTheVideoGame;
	case 0x47160288:
		return PentaneTarget::CarsMaterNationalChampionship;
	case 0x4DDE6899:
		return PentaneTarget::Cars2TheVideoGame;
	case 0x521E2EAF:
		return PentaneTarget::Cars2TheVideoGameArcade;
	case 0x58F94AF7:
		return PentaneTarget::Cars3DrivenToWin;
	default:
		return PentaneTarget::Invalid;
		break;
	}
}

auto get_offset_from_game(PentaneTarget target) -> std::uint32_t {
	switch (target) {
	case PentaneTarget::CarsTheVideoGame:
		return 0x36B234;
	case PentaneTarget::CarsMaterNationalChampionship:
		return 0x2E26E0;
	case PentaneTarget::Cars2TheVideoGame:
		return 0x11FE0AC;
	case PentaneTarget::Cars2TheVideoGameArcade:
		return 0x1447C5C;
	default:
		return 0;
	}
}

int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t* lpCmdLine, int nShowCmd) {
	wchar_t buffer[2048] = {};
	OPENFILENAMEW open = {};
	open.lStructSize = sizeof(OPENFILENAMEW);
	open.lpstrFilter = L"Executable Files (*.exe)\0*.exe";
	open.lpstrTitle = L"Select your game executable file...";
	open.lpstrFile = buffer;
	open.nMaxFile = sizeof(buffer) - 1;
	open.nFilterIndex = 1;
	open.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (GetOpenFileNameW(&open) != 0) {
		HANDLE file = CreateFileW(buffer, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file != nullptr) {
			IMAGE_DOS_HEADER dos_header{};
			DWORD bytes_read_written{};
			if (ReadFile(file, reinterpret_cast<void*>(&dos_header), sizeof(IMAGE_DOS_HEADER), &bytes_read_written, nullptr) != 0) {
				if (SetFilePointer(file, dos_header.e_lfanew, 0, FILE_BEGIN) != INVALID_SET_FILE_POINTER) {
					IMAGE_NT_HEADERS nt_header{};
					if (ReadFile(file, reinterpret_cast<void*>(&nt_header), sizeof(IMAGE_NT_HEADERS), &bytes_read_written, nullptr) != 0) {
						PentaneTarget game = get_target_from_timestamp(nt_header.FileHeader.TimeDateStamp);
						if (game != PentaneTarget::Invalid && game != PentaneTarget::Cars3DrivenToWin) {
							std::uint32_t offset = get_offset_from_game(game);
							if (SetFilePointer(file, offset, 0, FILE_BEGIN) != INVALID_SET_FILE_POINTER) {
								const char string[] = "Pentane.dll";
								WriteFile(file, &string, sizeof(string), &bytes_read_written, nullptr);
								CloseHandle(file);
								MessageBoxW(nullptr, L"Successfully installed Pentane!", L"Notice", MB_ICONINFORMATION);
							}
						}
						else {
							MessageBoxW(nullptr, L"Failed to detect game! Are you sure you are using a supported game/version?", L"Error", MB_ICONWARNING);
						}
					}
					else {
						CloseHandle(file);
					}
				}
				else {
					CloseHandle(file);
				}
			}
			else {
				CloseHandle(file);
			}
		}
		else {
			MessageBoxW(nullptr, L"Failed to open file! Are you sure you have the right permissions?", L"Error", MB_ICONWARNING);
		}
	}
}
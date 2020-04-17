#include "MKS80_LegacyBankLoader.h"

#include "MKS80.h"
#include "MKS80_Patch.h"

namespace midikraft {

	std::string MKS80_LegacyBankLoader::readPascalString(std::vector<uint8>::iterator &position, std::vector<uint8>::iterator const &end)
	{
		std::string name;
		if (position != end) {
			// Read length of string
			size_t len = *(position++);
			while (position != end && *position <= 0x20) len = *(position++);
			while (name.size() < len && position != end) {
				name.push_back((char) *(position++));
			}
		}
		return name;
	}

	std::vector<uint8> MKS80_LegacyBankLoader::readBinaryBlock(std::vector<uint8>::iterator &position, std::vector<uint8>::iterator const &end, int sizeToRead) {
		std::vector<uint8> result;
		if (std::distance(position, end) > sizeToRead - 1) {
			std::copy(position, position + sizeToRead, std::back_inserter(result)); 
			position += sizeToRead;
		}
		return result;
	}

	midikraft::TPatchVector MKS80_LegacyBankLoader::loadM80File(std::vector<uint8> fileContent)
	{
		// Load this old bank format floating around in the Internet - these are really just the DAT stream data with patch and tone names
		std::vector<std::vector<uint8>> toneDatas;
		std::vector<std::string> toneNames;
		std::vector<std::vector<uint8>> patchDatas;
		std::vector<std::string> patchNames;

		auto nextByte = fileContent.begin();
		while (nextByte != fileContent.end()) {
			std::string patch = readPascalString(nextByte, fileContent.end());
			std::vector<uint8> patchData = readBinaryBlock(nextByte, fileContent.end(), 0x17); // 0x17 bytes (23 dec) is one patch in the DAT format
			std::string tone = readPascalString(nextByte, fileContent.end());
			std::vector<uint8> toneData = readBinaryBlock(nextByte, fileContent.end(), 0x27); // 0x27 bytes (39 dec) is one tone in the DAT format
			std::copy(toneData.begin(), toneData.end(), std::back_inserter(patchData));
			if (patchData.size() == 0x17 + 0x27) {
				patchDatas.push_back(MKS80_Patch::patchesFromDat(patchData));
				patchNames.push_back(patch);
				toneDatas.push_back(MKS80_Patch::toneFromDat(toneData));
				toneNames.push_back(tone);
			}
			else {
				// Premature end of file
				SimpleLogger::instance()->postMessage("Aborting load of M80 file, file appears to be truncated");
				return {};
			}
		}

		if (toneDatas.size() == 64 && patchDatas.size() == 64) {
			// This was a complete load
			//TODO - how to transfer the patch and tone names?
			return MKS80::patchesFromAPRs(toneDatas, patchDatas);
		}
		else {
			SimpleLogger::instance()->postMessage("Did not find 64 patches and 64 tones, are you sure this is an M80 bank file?");
			return {};
		}
	}

	midikraft::TPatchVector MKS80_LegacyBankLoader::loadMKS80File(std::vector<uint8> fileContent)
	{
		if (fileContent.size() != 0xf80) {
			SimpleLogger::instance()->postMessage("File length is not 0xf80, this does not seem to be a proper mks80 file");
			return {};
		}

		std::vector<std::vector<uint8>> toneDatas;
		std::vector<std::vector<uint8>> patchDatas;
		auto nextByte = fileContent.begin();
		for (int i = 0; i < 64; i++) {
			auto datRow = readBinaryBlock(nextByte, fileContent.end(), 0x17 + 0x27); // 0x17 bytes (23 dec) is one patch in the DAT format and 0x27 bytes (39 dec) is one tone in the DAT format
			patchDatas.push_back(MKS80_Patch::patchesFromDat(datRow)); 
			toneDatas.push_back(MKS80_Patch::toneFromDat(datRow));
		}
		return MKS80::patchesFromAPRs(toneDatas, patchDatas);
	}

}



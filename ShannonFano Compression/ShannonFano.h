#pragma once

class ShannonFano
{
public:
	static float compressor(const char* filePath, const char* fileName);
	static void decompressor(const char* filePath, const char* fileName);

private:
	ShannonFano() {};
};


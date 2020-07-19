#include "ShannonFano.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

class BinWriter {
public:
	int k;
	ofstream f;
	char x;

	BinWriter(const char* p) : k(0) {
		f.open(p, ios::binary);
	}

	~BinWriter() {
		if (k > 0) writeByte(x);
		f.close();
	}

	void writeByte(char x) {
		f.write((char*)&x, 1);
	}

	void writeInt(int y) {
		f.write((char*)&y, 4);
	}

	void writeFloat(float y) {
		f.write((char*)&y, 4);
	}

	void writeBit(bool b) {
		if (k == 8) {
			writeByte(x);
			k = 0;
		}
		x ^= (-b ^ x) & (1 << k);
		k++;
	}
};

class BinReader {
public:
	int k;
	ifstream f;
	char x;

	BinReader(const char* p) : k(0) {
		f.open(p, ios::binary);
	}

	char readByte() {
		f.read((char*)&x, 1);
		return x;
	}

	int readInt() {
		int i;
		f.read((char*)&i, 4);
		return i;
	}

	float readFloat() {
		float fl;
		f.read((char*)&fl, 4);
		return fl;
	}

	bool readBit() {
		if (k == 8) {
			readByte();
			k = 0;
		}
		bool b = (x >> k) & 1;
		k++;
		return b;
	}
};

struct frequencyComponent
{
	unsigned char byte;
	int frequency;
	vector<bool> code;
};

struct moreThanFrequency
{
	inline bool operator() (const frequencyComponent& struct1, const frequencyComponent& struct2)
	{
		return (struct1.frequency > struct2.frequency);
	}
};

struct lessThanByte
{
	inline bool operator() (const frequencyComponent& struct1, const frequencyComponent& struct2)
	{
		return (struct1.byte < struct2.byte);
	}
};

vector<frequencyComponent> calculateFrequencies(const char* fileName)
{
	BinReader br(fileName);
	vector<frequencyComponent> frequencies;

	unsigned char c = br.readByte();

	frequencyComponent newComponent = { c, 1 };
	frequencies.push_back(newComponent);
	while (true)
	{
		c = br.readByte();
		if (br.f.eof()) break;

		for (int i = 0; i < frequencies.size(); i++)
		{
			if (frequencies[i].byte == c)
			{
				frequencies[i].frequency++;
				break;
			}

			if (i == frequencies.size() - 1)
			{
				frequencyComponent newComponent = { c, 1 };
				frequencies.push_back(newComponent);
				break;
			}
		}
	}

	std::sort(frequencies.begin(), frequencies.end(), lessThanByte());
	std::sort(frequencies.begin(), frequencies.end(), moreThanFrequency());

	return frequencies;
}

int getSumInRange(vector<frequencyComponent>& frequencies, int start, int end)
{
	int sum = 0;

	for (int i = start; i < end + 1; i++)
	{
		sum += frequencies[i].frequency;
	}

	return sum;
}

void shannonFanoTable(vector<frequencyComponent>& frequencies, int start, int end)
{
	if (end - start == 0)
	{
		return;
	}
	else
	{
		int rightSum = getSumInRange(frequencies, start, end);
		int leftSum = 0;

		leftSum += frequencies[0].frequency;
		rightSum -= frequencies[0].frequency;
		int diff = abs(leftSum - rightSum);

		int divider;
		for (int i = start + 1; i < end + 1; i++)
		{
			leftSum += frequencies[i].frequency;
			rightSum -= frequencies[i].frequency;
			int nextDiff = abs(leftSum - rightSum);
			if (nextDiff >= diff)
			{
				divider = i;
				break;
			}
			else
			{
				diff = nextDiff;
			}
		}

		for (int i = start; i < divider; i++)
		{
			frequencies[i].code.push_back(0);
		}

		for (int i = divider; i < end + 1; i++)
		{
			frequencies[i].code.push_back(1);
		}

		shannonFanoTable(frequencies, start, divider - 1);
		shannonFanoTable(frequencies, divider, end);
	}
}

struct treeNode
{
	unsigned char byte;
	bool isLeaf = false;

	treeNode* leftNode;
	treeNode* rightNode;
};

treeNode* shannonFanoDecompress(vector<frequencyComponent>& frequencies, int start, int end)
{
	treeNode* newNode = new treeNode;

	if (end - start == 0)
	{
		newNode->byte = frequencies[start].byte;
		newNode->isLeaf = true;
	}
	else
	{
		int rightSum = getSumInRange(frequencies, start, end);
		int leftSum = 0;

		leftSum += frequencies[0].frequency;
		rightSum -= frequencies[0].frequency;
		int diff = abs(leftSum - rightSum);

		int divider;
		for (int i = start + 1; i < end + 1; i++)
		{
			leftSum += frequencies[i].frequency;
			rightSum -= frequencies[i].frequency;
			int nextDiff = abs(leftSum - rightSum);
			if (nextDiff >= diff)
			{
				divider = i;
				break;
			}
			else
			{
				diff = nextDiff;
			}
		}

		newNode->leftNode = shannonFanoDecompress(frequencies, start, divider - 1);
		newNode->rightNode = shannonFanoDecompress(frequencies, divider, end);
	}

	return newNode;
}

unsigned char decodeByte(treeNode* node, int& decodedBytesLength, BinReader& br)
{
	if (node->isLeaf == true)
	{
		return node->byte;
	}

	bool b = br.readBit();

	decodedBytesLength++;
	if (b == 0) decodeByte(node->leftNode, decodedBytesLength, br);
	else decodeByte(node->rightNode, decodedBytesLength, br);
}

int calculateMessageSize(vector<frequencyComponent> frequencies)
{
	int size = 0;
	for (int i = 0; i < frequencies.size(); i++)
	{
		size += frequencies[i].frequency * frequencies[i].code.size();
	}
	return size;
}

int calculateInitialFileSize(vector<frequencyComponent> frequencies)
{
	int size = 0;
	for (int i = 0; i < frequencies.size(); i++)
	{
		size += frequencies[i].frequency * 8;
	}
	return size;
}

float ShannonFano::compressor(const char* filePath, const char* fileName)
{
	vector<frequencyComponent> frequencies;

	frequencies = calculateFrequencies(filePath);
	shannonFanoTable(frequencies, 0, frequencies.size() - 1);
	int initialFileSize = calculateInitialFileSize(frequencies);

	BinWriter bw(fileName);
	vector<int> outTable(256, 0);

	for (int i = 0; i < frequencies.size(); i++)
	{
		outTable[(int)frequencies[i].byte] = frequencies[i].frequency;
	}

	for (int i = 0; i < outTable.size(); i++)
	{
		bw.writeInt(outTable[i]);
	}

	BinReader br(filePath);
	unsigned char c = br.readByte();
	for (int i = 0; i < frequencies.size(); i++)
	{
		if (c == frequencies[i].byte)
		{
			for (int j = 0; j < frequencies[i].code.size(); j++)
			{
				bw.writeBit(frequencies[i].code[j]);
			}
			break;
		}
	}
	int bitsWritten = 1;
	while (true)
	{
		c = br.readByte();
		if (br.f.eof()) break;

		for (int i = 0; i < frequencies.size(); i++)
		{
			if (c == frequencies[i].byte)
			{
				for (int j = 0; j < frequencies[i].code.size(); j++)
				{
					bw.writeBit(frequencies[i].code[j]);
					bitsWritten++;
				}
				break;
			}
		}
	}

	float compressionRate = ((float)initialFileSize) / (float)bitsWritten;
	return compressionRate;
}

void ShannonFano::decompressor(const char* filePath, const char* fileName)
{
	vector<frequencyComponent> frequencies;

	BinReader br(filePath);
	for (int i = 0; i < 256; i++)
	{
		int frequency = br.readInt();
		if (frequency != 0)
		{
			frequencyComponent newComponent = { char(i), frequency };
			frequencies.push_back(newComponent);
		}
	}

	std::sort(frequencies.begin(), frequencies.end(), lessThanByte());
	std::sort(frequencies.begin(), frequencies.end(), moreThanFrequency());
	shannonFanoTable(frequencies, 0, frequencies.size() - 1);
	treeNode* root = shannonFanoDecompress(frequencies, 0, frequencies.size() - 1);
	int messageSize = calculateMessageSize(frequencies);
	int decodedBytesLength = 0;

	br.readByte();

	BinWriter bw(fileName);
	while (decodedBytesLength < messageSize)
	{
		unsigned char c = decodeByte(root, decodedBytesLength, br);
		bw.writeByte(c);
	}
}
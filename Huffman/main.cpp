#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <map>
#include <vector>
#include <algorithm>
#include <set>
#include <queue>
using namespace std;
#endif /* __PROGTEST__ */

//#include "tests/stopwatch.h"
struct Node
{
	Node()
		: left(NULL), right(NULL),c('0')
	{};

	char c;
	Node* left;
	Node* right;
	bool leaf = false;
};

class HuffTree
{
public:
	~HuffTree()
	{
		deleteTree(m_Root);
	}
	bool MoveInTree(const char c, char& tmp)
	{
		if (!m_Traversal)
			m_Traversal = m_Root;
		
		if (c == '0')
		{
			//std::cout << "Moving left" << std::endl;
			m_Traversal = m_Traversal->left;

			if (m_Traversal != NULL && m_Traversal->leaf)
			{		
				tmp = m_Traversal->c; 
				m_Traversal = m_Root;
				return true;
			}
		}
		else
		{
			//std::cout << "Moving right" << std::endl;
			m_Traversal = m_Traversal->right;
			if (m_Traversal != NULL && m_Traversal->leaf)
			{
				tmp = m_Traversal->c;  
				m_Traversal = m_Root;
				return true;
			}
		}
		return false;
	}
	const char FindCode(const std::string& path)
	{
		return findCode(m_Root, path, 0);
	}
	size_t CreateBranch(const std::string& path)
	{
		size_t index = 0;
		createBranch(&m_Root, path, index);
		return index;
	}

private:
	void deleteTree(Node* node)
	{
		if (node == NULL) return;

		/* first delete both subtrees */
		deleteTree(node->left);
		deleteTree(node->right);

		free(node);
	}
	const char findCode(Node* node, const std::string& path, size_t index)
	{
		if (path.size() == index)
		{
			return node->c;
		}
		else if (path[index] == '0')
		{	
			return findCode(node->left, path, index + 1);
		}
		else
		{
			return findCode(node->right, path, index + 1);
		}
	}
	void createBranch(Node** node, const std::string& path, size_t& index)
	{
		if (index == path.size())
			return;

		if ((*node) == NULL)
		{
			(*node) = new Node;
		}

		if (path[index] == '0')
		{
			index++;
			//std::cout << "Creating internal node" << std::endl;
			
			createBranch(&(*node)->left, path, index);
			createBranch(&(*node)->right, path, index);
			
			return;
		}
		else if (path[index] == '1')
		{
			(*node)->leaf = true;
			(*node)->c = (char)strtol(path.substr(index + 1, 8).c_str(), 0, 2);
			index += 9;
		}
		return;
	}

private:
	Node* m_Root;
	Node* m_Traversal;

};



class HuffFile
{
public:
	HuffFile()
		:m_Tree(HuffTree())
	{}

	bool decompress(const char* inFileName, const char* outFileName)
	{
		ifstream in;
		in.open(inFileName, ios::in | ios::binary);
		std::string lol;
		if (!in.is_open())
		{
			return false;
		}
		if (in.good())
		{
			char c;
			while (in.get(c))
			{
				for (int i = 7; i >= 0; i--) // or (int i = 0; i < 8; i++)  if you want reverse bit order in bytes
				{
					int num = ((c >> i) & 1);
					lol += to_string(num);
					//ostr << num;
				}
			}
		}
		else
		{
			return false;
		}
		
		//ostr << lol;
		if (lol.size() % 8 != 0)
			return false;

		in.close();
		if (in.is_open())
			return false;

		size_t counter = 0;
		//std::cout<<"Size " << ostr.str().size() << std::endl;
		{
			//Stopwatch watch;
			counter = m_Tree.CreateBranch(lol);
		}
		//std::cout << counter << std::endl;


		bool pleaseFalse = false;
		int countChars = 0;
		char tmp;
		ofstream out;
		out.open(outFileName, ios::out | ios::binary);
		if (!out.is_open())
			return false;
		if (!out.good())
			return false;
 
		string answer;
		size_t size = lol.size();
		{
			//Stopwatch watch;
			if (lol[counter] == '0')
			{
				m_NumChars = binaryToDecimal(lol.substr(counter + 1, 12));
				counter += 13;
				
				while (counter < size)
				{
					if (m_Tree.MoveInTree(lol[counter], tmp))
					{
						out << tmp;
						countChars++;
						if (countChars == (int)m_NumChars)
							break;
					}
					++counter;
				}
			}
			else
			{
				if (size < 4096)
					return false;
				while (counter < size)
				{
					counter++;
					int temp = 0;

					while (temp < 4096)
					{
						if (m_Tree.MoveInTree(lol[counter], tmp))
						{
							{
								//Stopwatch watch;
								//out << tmp;
								answer += tmp;
							}
							temp++;
							countChars++;
							m_NumChars++;
						}
						counter++;
					}
					if (lol[counter] == '0')
					{
						int test = binaryToDecimal(lol.substr(counter + 1, 12));
						if (test == 0)
							break;

						m_NumChars += test;
						counter += 13;

						while (counter < size)
						{
							if (m_Tree.MoveInTree(lol[counter], tmp))
							{
								if (tmp > 127)
								{
									pleaseFalse = true;
									break;
								}
								answer += tmp;
								//out << tmp;
								countChars++;
							}
							++counter;
						}
						break;
					}
				}
			}
		}

		out << answer;
		if (countChars != (int)m_NumChars)
			return false;
		
		out.close();
		if (out.is_open())
			return false;

		if (pleaseFalse)
			return false;
		//std::cout << "Its allright lol" << std::endl;
		return true;
	}
	int binaryToDecimal(const string& n)
	{
		string num = n;
		int dec_value = 0;

		int base = 1;

		int len = num.length();
		for (int i = len - 1; i >= 0; i--)
		{
			if (num[i] == '1')
				dec_value += base;
			base = base * 2;
		}
		return dec_value;
	}
private:
	HuffTree m_Tree;
	size_t m_NumChars = 0;
};


bool decompressFile(const char* inFileName, const char* outFileName)
{
	HuffFile file;
	return file.decompress(inFileName, outFileName);
}

bool compressFile(const char* inFileName, const char* outFileName)
{
	// keep this dummy implementation (no bonus) or implement the compression (bonus)
	return false;
}
#ifndef __PROGTEST__
bool identicalFiles(const char* fileName1, const char* fileName2)
{
	std::ifstream lFile(fileName1, std::ifstream::in | std::ifstream::binary);
	std::ifstream rFile(fileName2, std::ifstream::in | std::ifstream::binary);

	if (!lFile.is_open() || !rFile.is_open())
	{
		return false;
	}

	char* lBuffer = new char[1000000]();
	char* rBuffer = new char[1000000]();

	do
	{
		lFile.read(lBuffer, 1000000);
		rFile.read(rBuffer, 1000000);

		if (std::memcmp(lBuffer, rBuffer, 1000000) != 0)
		{
			delete[] lBuffer;
			delete[] rBuffer;
			return false;
		}
	} while (lFile.good() || rFile.good());

	delete[] lBuffer;
	delete[] rBuffer;
	return true;
}

int main(void)
{
	{
		//Stopwatch watch;
		assert(decompressFile("tests/test0.huf", "tempfile"));
		assert(identicalFiles("tests/test0.orig", "tempfile"));

		assert(decompressFile("tests/test1.huf", "tempfile"));
		assert(identicalFiles("tests/test1.orig", "tempfile"));

		assert(decompressFile("tests/test2.huf", "tempfile"));
		assert(identicalFiles("tests/test2.orig", "tempfile"));

		assert(decompressFile("tests/test3.huf", "tempfile"));
		assert(identicalFiles("tests/test3.orig", "tempfile"));

		assert(decompressFile("tests/test4.huf", "tempfile"));
		assert(identicalFiles("tests/test4.orig", "tempfile"));

		assert(!decompressFile("tests/test5.huf", "tempfile"));

		assert(decompressFile("in_3500676.bin", "lol.txt"));
		assert(!decompressFile("in_3500747.bin", "lol2.txt"));
	}
	//assert(decompressFile("tests/extra0.huf", "tempfile"));
	//assert(identicalFiles("tests/extra0.orig", "tempfile"));
	//
	//assert(decompressFile("tests/extra1.huf", "tempfile"));
	//assert(identicalFiles("tests/extra1.orig", "tempfile"));
	//
	//assert(decompressFile("tests/extra2.huf", "tempfile"));
	//assert(identicalFiles("tests/extra2.orig", "tempfile"));
	//
	//assert(decompressFile("tests/extra3.huf", "tempfile"));
	//assert(identicalFiles("tests/extra3.orig", "tempfile"));
	//
	//assert(decompressFile("tests/extra4.huf", "tempfile"));
	//assert(identicalFiles("tests/extra4.orig", "tempfile"));
	//
	//assert(decompressFile("tests/extra5.huf", "tempfile"));
	//assert(identicalFiles("tests/extra5.orig", "tempfile"));
	//
	//assert(decompressFile("tests/extra6.huf", "tempfile"));
	//assert(identicalFiles("tests/extra6.orig", "tempfile"));
	//
	//assert(decompressFile("tests/extra7.huf", "tempfile"));
	//assert(identicalFiles("tests/extra7.orig", "tempfile"));
	//
	//assert(decompressFile("tests/extra8.huf", "tempfile"));
	//assert(identicalFiles("tests/extra8.orig", "tempfile"));
	//
	//assert(decompressFile("tests/extra9.huf", "tempfile"));
	//assert(identicalFiles("tests/extra9.orig", "tempfile"));

	return 0;
}
#endif /* __PROGTEST__ */

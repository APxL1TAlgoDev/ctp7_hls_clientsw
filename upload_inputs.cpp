#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <CTP7AlgoClient.hh>


#include <stdexcept>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <map>

std::string lut_file_path;

typedef struct
{
	std::vector<uint32_t> data;
} input_data_t;

std::map<int, input_data_t> load_file(std::string path)
{
	std::ifstream infile(path.c_str(), std::fstream::in);

	if (!infile.is_open())
		throw std::runtime_error(std::string("Unable to open input file: ") + path);

	// std map container to store complete LUT info read from the file
	// 28x8182 uint32_t for 2nd Stage LUTs (iEta=[1-28])
	std::map<int, input_data_t> input_data;

	uint32_t value;


	std::string dummyLine;

	// skip header lines
	getline(infile, dummyLine);
	getline(infile, dummyLine);
	getline(infile, dummyLine);

	// Read input data from the file
	for (int idx = 0; idx < 1024; idx++)
	{
		infile >> std::hex >> value;

		for (int i = 0; i <= 66; i++)
		{
			infile >> std::hex >> value;

			input_data[i].data.push_back(value);
		}
	}

	return input_data;
}

void upload_inputs(CTP7AlgoClient &card, int phi)
{

	std::map<int, input_data_t> input_data;

	try
	{
		char namepart[64];

		// LUT File
		snprintf(namepart, 64, "input_link_data.txt");

		// Read info from the LUT file
		input_data = load_file(lut_file_path + "/" + namepart);
	} 
	catch (std::runtime_error &e)
	{
		printf("Error loading link data for phi %d: %s\n", phi, e.what());
		return ;
	}


		for (int link = 0; link <= 66; link++)
		{
			card.setInputLinkBuffer(link, input_data[link].data);
		}
	

	return;



}

int main(int argc, char *argv[])
{
	std::vector<CTP7AlgoClient*> cards;

	mkdir(argv[2], 0777); // if it fails, it fails.
	char *realpattern = realpath(argv[2], NULL);
	if (!realpattern)
	{
		printf("Unable to access pattern output directory.\n");
		return 1;
	}
	lut_file_path = realpattern;
	free(realpattern);

	int phi = atoi(argv[1]);

	CTP7AlgoClient * card;

	try
	{
		card = new CTP7AlgoClient(phi, "CTP7Map.xml");
	}
	catch (std::runtime_error &e)
	{
		printf("Couldnt connect to phi %d: %s\n", phi, e.what());
	}

	upload_inputs(*card, phi);

	return 0;
}

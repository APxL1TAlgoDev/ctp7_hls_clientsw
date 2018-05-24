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

void download_outputs(CTP7AlgoClient &card, int phi)
{


	card.restartPattern();

	std::map<int, std::vector<uint32_t> > output_link_data;

	for (int link = 0; link <= 47; link++)
	{
		card.getOutputLinkBuffer(link, output_link_data[link]);
	}


	char filename[64];
	snprintf(filename, 64, "output_link_data.txt");
	FILE *fd = fopen((lut_file_path + "/" + filename).c_str(), "w");
	if (!fd)
	{
		printf("Error writing output file %s\n", filename);
		return ;
	}

	fprintf(fd, "====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================== \n");
	fprintf(fd, "Output       LINK_00       LINK_01       LINK_02       LINK_03       LINK_04       LINK_05       LINK_06       LINK_07       LINK_08       LINK_09       LINK_10       LINK_11       LINK_12       LINK_13       LINK_14       LINK_15       LINK_16       LINK_17       LINK_18       LINK_19       LINK_20       LINK_21       LINK_22       LINK_23       LINK_24       LINK_25       LINK_26       LINK_27       LINK_28       LINK_29       LINK_30       LINK_31       LINK_32       LINK_33       LINK_34       LINK_35       LINK_36       LINK_37       LINK_38       LINK_39       LINK_40       LINK_41       LINK_42       LINK_43       LINK_44       LINK_45       LINK_46       LINK_47 \n");
	fprintf(fd, "====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================== \n");




	for (int word = 0; word < 1024; word++)
	{
		fprintf(fd, "0x%05x   ", word);
		for (int link = 0; link <= 47; link++)
		{
			fprintf(fd, "0x%08x    ", output_link_data[link][word]);
		}
		fprintf(fd, "\n");
	}

	fprintf(fd, "====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================== \n");


	fclose(fd);

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

	download_outputs(*card, phi);

	return 0;
}

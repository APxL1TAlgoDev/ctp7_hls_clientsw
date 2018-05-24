#include <stdio.h>
#include <stdexcept>
#include <wiscrpcsvc.h>
#include "CTP7AlgoClient.hh"

// Including the files included by tinyxml2.h here as well, to make sure we
// don't get them subsequently imported with the wrong visibility and having it
// matter somehow, later.  (Hail header include guards!)
#include <cctype>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#pragma GCC visibility push(hidden)
#include "tinyxml2.h"
#pragma GCC visibility pop

using namespace wisc;

static const std::string module_name = "CTP7HLS";
static const std::string module_key = "CTP7HLS v1.1.0";

#define RPC_CATCH_RETURN catch (wisc::RPCSvc::RPCException &e) {  return false; }

#define TYPE_GENERIC_CALL(rtype, result, rpcreq) \
    do { \
        if (!this->checkConnection()) \
            return false; \
        try { \
            RPCMsg rpcrsp = this->rpc.call_method(rpcreq); \
            if (rpcrsp.get_key_exists("error")) \
                return false; \
            if (rpcrsp.get_key_exists("result")) \
                result = rpcrsp.get_ ## rtype("result"); \
            return true; \
        } \
        RPC_CATCH_RETURN; \
    } while (0)

#define WORD_GENERIC_CALL(result, rpcreq) TYPE_GENERIC_CALL(word, result, rpcreq)
#define GENERIC_CALL(rpcreq) WORD_GENERIC_CALL(__attribute__((unused)) uint32_t discard, rpcreq)

CTP7AlgoClient::CTP7AlgoClient(uint32_t phi, const std::string connect_string, enum ConnectStringType connect_type)
	: phi(phi)
{
	// Find out the correct address to access.

	if (connect_type == CONNECTSTRING_PHIMAPXML) {
		tinyxml2::XMLDocument phiMapDoc;
		if (phiMapDoc.LoadFile(connect_string.c_str()))
			throw std::runtime_error("Unable to read address configuration XML");

		tinyxml2::XMLElement *config_elem = phiMapDoc.FirstChildElement("layer1_hw_config");
		if (!config_elem)
			throw std::runtime_error("Unable to read address configuration XML");

		this->hostname = "";

		for (tinyxml2::XMLElement *phi_elem = config_elem->FirstChildElement("phi_config"); phi_elem; phi_elem = phi_elem->NextSiblingElement("phi_config"))
		{
			int phival;
			if (phi_elem->QueryIntAttribute("phi", &phival) != 0)
				continue;
			if (phival != phi)
				continue;
			tinyxml2::XMLElement *ba_elem = phi_elem->FirstChildElement("board_address");
			if (!ba_elem)
				continue;
			const char *ba_text = ba_elem->GetText();
			if (!ba_text)
				continue;
			this->hostname = ba_text;
			break;
		}

		if (!this->hostname.size())
			throw std::runtime_error("Unable to locate address configuration for the requested phi");
	}
	else if (connect_type == CONNECTSTRING_IPHOST) {
		this->hostname = connect_string;
	}
	else {
		throw std::runtime_error("Invalid connect_type provided");
	}

	try
	{
		this->rpc.connect(this->hostname);
		if (!this->rpc.load_module(module_name, module_key))
			throw std::runtime_error("Unable to load the appropriate rpcsvc module");
	}
	catch (wisc::RPCSvc::RPCException &e)
	{
		throw std::runtime_error(std::string("RPCSvc Exception: ") + e.message);
	}
}


CTP7AlgoClient::~CTP7AlgoClient()
{

}

bool CTP7AlgoClient::checkConnection()
{
	try
	{
		this->rpc.call_method(RPCMsg(module_name + ".ping"));
		return true;
	}
	catch (wisc::RPCSvc::RPCException &e)
	{
		//
	}

	// Bad connection.  Let's try reconnecting.
	this->rpc.disconnect();

	try
	{
		this->rpc.connect(this->hostname);
		if (!this->rpc.load_module(module_name, module_key))
			return false;
		return true;
	}
	catch (wisc::RPCSvc::RPCException &e)
	{
		return false;
	}
}

bool CTP7AlgoClient::hardReset(std::string bitstream)
{
	GENERIC_CALL(RPCMsg(module_name + ".hardReset")
	             .set_string("bitstream", bitstream)
	             .set_word("phi", this->phi)
	            );
}

bool CTP7AlgoClient::softReset()
{
	GENERIC_CALL(RPCMsg(module_name + ".softReset"));
}


bool CTP7AlgoClient::getConfiguration(std::string &output)
{
	TYPE_GENERIC_CALL(string, output, RPCMsg(module_name + ".getConfiguration"));
}

bool CTP7AlgoClient::setConfiguration(std::string input)
{
	GENERIC_CALL(RPCMsg(module_name + ".setConfiguration")
	             .set_string("input", input)
	            );
}


bool CTP7AlgoClient::getInputLinkBuffer(int link, std::vector<uint32_t> &selectedData)
{
	TYPE_GENERIC_CALL(word_array, selectedData,
	                  RPCMsg(module_name + ".getInputLinkBuffer")
	                  .set_word("link", link)
	                 );
}

bool CTP7AlgoClient::setInputLinkBuffer( int link, const std::vector<uint32_t> &selectedData)
{
	GENERIC_CALL(RPCMsg(module_name + ".setInputLinkBuffer")
	             .set_word("link", link)
	             .set_word_array("selectedData", selectedData)
	            );
}


bool CTP7AlgoClient::getOutputLinkBuffer(int link, std::vector<uint32_t> &selectedData)
{
	TYPE_GENERIC_CALL(word_array, selectedData,
	                  RPCMsg(module_name + ".getOutputLinkBuffer")
	                  .set_word("link", link)
	                 );
}

bool CTP7AlgoClient::setOutputLinkBuffer( int link, const std::vector<uint32_t> &selectedData)
{
	GENERIC_CALL(RPCMsg(module_name + ".setOutputLinkBuffer")
	             .set_word("link", link)
	             .set_word_array("selectedData", selectedData)
	            );
}

bool CTP7AlgoClient::restartPattern()
{
	GENERIC_CALL(RPCMsg(module_name + ".restartPattern"));
}


bool CTP7AlgoClient::getConfigReg( int reg, int & value)
{
	TYPE_GENERIC_CALL(word, value,
	                  RPCMsg(module_name + ".getConfigReg")
	                  .set_word("reg", reg)
	                 );
}

bool CTP7AlgoClient::setConfigReg( int reg, int value)
{
	GENERIC_CALL(RPCMsg(module_name + ".setConfigReg")
	             .set_word("reg", reg)
	             .set_word("value", value)
	            );
}


/*
 *  typedef struct FWInfo
 *  {
 *      uint32_t buildTimestamp;
 *      uint32_t gitHashCode;
 *      uint32_t gitHashDirty;
 *      uint32_t version;
 *      uint32_t projectCode;
 *  } FWInfo;
*/

bool CTP7AlgoClient::getFWInfo(FWInfo &fwInfo)
{

	if (!this->checkConnection())
		return false;
	try
	{
		RPCMsg rpcrsp = this->rpc.call_method(
		                    RPCMsg(module_name + ".getFWInfo")
		                    .set_word("dummy", 0)
		                );
		if (rpcrsp.get_key_exists("error"))
			return false;

		if (!rpcrsp.get_key_exists("buildTimestamp"))    return false;
		if (!rpcrsp.get_key_exists("gitHashCode"))       return false;
		if (!rpcrsp.get_key_exists("gitHashDirty"))      return false;
		if (!rpcrsp.get_key_exists("version"))           return false;
		if (!rpcrsp.get_key_exists("projectCode"))       return false;
		if (!rpcrsp.get_key_exists("uptime"))            return false;

		fwInfo.buildTimestamp    = rpcrsp.get_word("buildTimestamp");
		fwInfo.gitHashCode       = rpcrsp.get_word("gitHashCode");
		fwInfo.gitHashDirty      = rpcrsp.get_word("gitHashDirty");
		fwInfo.version           = rpcrsp.get_word("version");
		fwInfo.projectCode       = rpcrsp.get_word("projectCode");

		return true;
	}
	RPC_CATCH_RETURN;
}

bool CTP7AlgoClient::getModuleBuildInfo(std::string &value)
{
	TYPE_GENERIC_CALL(string, value, RPCMsg(module_name + ".getModuleBuildInfo"));
}


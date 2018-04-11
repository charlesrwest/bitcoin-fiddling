#include<iostream>
#include <bitcoin/bitcoin.hpp>
#include "commandLineArgumentParser.hpp"
#include<string>
#include<map>

using namespace Fiddling;
using namespace libbitcoin;

class WalletInfo
{
public:
    std::string FileName;
    std::string PublicKey;
    std::string PrivateKey;
    std::string Address;
};

class Datastore
{
public:
    std::string KeyFolder;
    std::map<std::string, WalletInfo> Wallets;
};

#if 0
std::string PublicKeyFromPrivateKey(const std::string& privateKey)
{
    ec_compressed point;
    secret_to_public(point, privateKey);

    return ec_public(point, true);
}
#endif


const int64_t EXPECTED_PRIVATE_KEY_SIZE_IN_HEX = 64;

int main(int argc, const char** argv)
{

//Parse arguments to get list of wallets to take money from and wallets to send money to (as well as amounts), as well as the folder where the wallets information is stored
commandLineParser parser;

parser.clampOptionNumberOfOptionArguments("wd", 1);
parser.addSuggestedUsage("wd", {"DirectoryToFindWalletKeys"});

parser.addSuggestedUsage("i", {"FileNameOfWalletKeys", "AmountToTakeInSatoshis"});

parser.addSuggestedUsage("o", {"FileNameOfWalletKeys", "AmountToReceiveInSatoshis"});

parser.clampOptionNumberOfOptionArguments("help", 0);
parser.addSuggestedUsage("help", {});

parser.parse(argv, argc);

if(parser.has({"help"}))
{
    parser.printHelpMessage();
    return 0;
}

int64_t error_count = 0;
for(const std::string& option : {"i", "o", "wd"})
{
    if(!parser.has(option))
    {
        parser.printErrorMessageRegardingOption(option);
        error_count++;
    }
}

if(error_count > 0)
{
    return 1;
}

std::map<std::string, int64_t> input_amounts;
std::map<std::string, int64_t> output_amounts;

Datastore wallets;
wallets.KeyFolder = parser.optionToAssociatedArguments.at("wd")[0];

const std::vector<std::string>& input_arguments = parser.optionToAssociatedArguments.at("i");

for(int64_t input_index = 0; input_index < input_arguments.size(); input_index++)
{
    if((input_index % 2) != 0)
    {
        continue;  //Skip amounts
    }

    const std::string& file_name = input_arguments[input_index];

    if(wallets.Wallets.count(file_name) > 0)
    {
        std::cerr << "We already have an input for wallet " << file_name << std::endl;
        return 1;
    }

    std::ifstream wallet_private_key_file(wallets.KeyFolder + "/" + file_name + ".private_key");

    std::string private_key;
    wallet_private_key_file >> private_key;

    if(private_key.size() != EXPECTED_PRIVATE_KEY_SIZE_IN_HEX)
    {
        std::cerr << "Unable to read private key for wallet " << file_name << std::endl;
        return 1;
    }

    WalletInfo& wallet = wallets.Wallets[file_name];
    wallet.PrivateKey = private_key;
    wallet.FileName = file_name;
    //wallet.PublicKey = ;

    std::cout << "Wallet " << file_name << " has private key " << private_key << std::endl;
}

for(const std::pair<const std::string, std::vector<std::string>>& option_and_arguments : parser.optionToAssociatedArguments)
{
    std::cout << option_and_arguments.first << " has arguments ";
    for(const std::string& argument : option_and_arguments.second)
    {
        std::cout << argument << " ";
    }
    std::cout << std::endl;
}

//Look up/store all transactions which each sender wallet can use by themselves, compute balance for each input wallet

//Check that input wallet balances are sufficient to cover their inputs to the transactions

//Choose a set of transactions to pull from for each input wallets (smallest first)

//Construct transaction

//Sign transaction

//Validate transaction

//Send transaction


libbitcoin::endorsement endorse;


return 0;
}

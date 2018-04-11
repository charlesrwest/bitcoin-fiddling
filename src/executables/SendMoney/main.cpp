#include<iostream>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client.hpp>
#include "commandLineArgumentParser.hpp"
#include<string>
#include<map>
#include <bitcoin/explorer/prop_tree.hpp>

using namespace Fiddling;
using namespace libbitcoin;

class WalletInfo
{
public:
    std::string FileName;
    libbitcoin::wallet::ec_public PublicKey;
    wallet::ec_private PrivateKey;
    wallet::payment_address Address;
};

class Datastore
{
public:
    std::string KeyFolder;
    std::map<std::string, WalletInfo> Wallets;
};

wallet::ec_private PrivateKeyFromHex(const std::string& privateKey)
{
    ec_secret raw_private_key;
    decode_base16(raw_private_key, privateKey);
    wallet::ec_private private_key(raw_private_key, 0x8000, false);

    return private_key;
}

libbitcoin::wallet::ec_public PublicKeyFromPrivateKey(const wallet::ec_private& privateKey)
{
    ec_compressed point;
    secret_to_public(point, privateKey);

    return wallet::ec_public(point, true);
}

bool ConvertStringToInteger(const std::string &inputString, int64_t &inputIntegerBuffer)
{
    char *stringEnd = nullptr;
    inputIntegerBuffer = strtol(inputString.c_str(), &stringEnd, 0);

    if(((const char *) stringEnd) == inputString.c_str())
    { //Couldn't convert correctly
        return false;
    }

    return true;
}

const int64_t EXPECTED_PRIVATE_KEY_SIZE_IN_HEX = 64;
const int64_t EXPECTED_PUBLIC_KEY_SIZE_IN_HEX = 66;

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

Datastore datastore;
datastore.KeyFolder = parser.optionToAssociatedArguments.at("wd")[0];

const std::vector<std::string>& input_arguments = parser.optionToAssociatedArguments.at("i");

std::string last_wallet_file_name = "";

if((input_arguments.size() % 2) != 0)
{
    std::cerr << "Expected an even number of input arguments" << std::endl;
    return 1;
}

for(int64_t input_index = 0; input_index < input_arguments.size(); input_index++)
{
    if((input_index % 2) != 0)
    {
        int64_t amount = -1;
        if(!ConvertStringToInteger(input_arguments[input_index], amount))
        {
            std::cerr << "Invalid amount :" << input_arguments[input_index] << std::endl;
            return 1;
        }

        input_amounts[last_wallet_file_name] += amount;
    }
    else
    {
        const std::string& file_name = input_arguments[input_index];

        if(datastore.Wallets.count(file_name) > 0)
        {
            std::cerr << "We already have an input for wallet " << file_name << std::endl;
            return 1;
        }

        std::ifstream wallet_private_key_file(datastore.KeyFolder + "/" + file_name + ".private_key");

        std::string private_key_hex;
        wallet_private_key_file >> private_key_hex;

        if(private_key_hex.size() != EXPECTED_PRIVATE_KEY_SIZE_IN_HEX)
        {
            std::cerr << "Unable to read private key for wallet " << file_name << std::endl;
            return 1;
        }

        WalletInfo& wallet = datastore.Wallets[file_name];
        wallet.PrivateKey = PrivateKeyFromHex(private_key_hex);
        wallet.FileName = file_name;
        wallet.PublicKey = PublicKeyFromPrivateKey(wallet.PrivateKey);
        wallet.Address = wallet::payment_address(wallet.PublicKey);

        last_wallet_file_name = file_name;
    }
}

const std::vector<std::string>& output_arguments = parser.optionToAssociatedArguments.at("o");
if((output_arguments.size() % 2) != 0)
{
    std::cerr << "Expected an even number of output arguments" << std::endl;
    return 1;
}

for(int64_t output_index = 0; output_index < input_arguments.size(); output_index++)
{
    if((output_index % 2) != 0)
    {
        int64_t amount = -1;
        if(!ConvertStringToInteger(input_arguments[output_index], amount))
        {
            std::cerr << "Invalid amount :" << output_arguments[output_index] << std::endl;
            return 1;
        }

        output_amounts[last_wallet_file_name] += amount;
    }
    else
    {
        const std::string& file_name = output_arguments[output_index];

        if(datastore.Wallets.count(file_name) > 0)
        {
            //We already have this wallet (presumably from the inputs), so skip
            last_wallet_file_name = file_name;
            continue;
        }

        std::ifstream wallet_public_key_file(datastore.KeyFolder + "/" + file_name + ".public_key");

        std::string public_key_hex;
        wallet_public_key_file >> public_key_hex;

        if(public_key_hex.size() != EXPECTED_PUBLIC_KEY_SIZE_IN_HEX)
        {
            std::cerr << "Unable to read public key for wallet " << file_name << std::endl;
            return 1;
        }

        WalletInfo& wallet = datastore.Wallets[file_name];
        wallet.FileName = file_name;
        wallet.PublicKey = wallet::ec_public(public_key_hex);
        wallet.Address = wallet::payment_address(wallet.PublicKey);

        last_wallet_file_name = file_name;
    }
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

client::connection_type connection = {};
connection.retries = 3;
connection.timeout_seconds = 8;
connection.server = config::endpoint("tcp://mainnet.libbitcoin.net:9091");

client::obelisk_client client(connection);

if(!client.connect(connection))
{
    std::cerr << "Error connecting to libbitcoin server" << std::endl;
    return 1;
}

chain::history::list history;



auto on_done = [&history](const chain::history::list& rows)
{
    history = rows;
};

auto on_error = [](const code& error)
{
    std::cerr << "Error retrieving from libbitcoin server" << std::endl;
};

// This does not include unconfirmed transactions.
client.blockchain_fetch_history3(on_error, on_done, datastore.Wallets.begin()->second.Address.hash());
client.wait();

//explorer::config::prop_tree history_tree(history, true);

//Check that input wallet balances are sufficient to cover their inputs to the transactions

//Choose a set of transactions to pull from for each input wallets (smallest first)

//Construct transaction

//Sign transaction

//Validate transaction

//Send transaction


libbitcoin::endorsement endorse;


return 0;
}

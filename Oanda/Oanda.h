#pragma once
#include <iostream>

#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <chrono>

using namespace Poco;
using namespace Poco::Net;
using namespace std;

static const std::string accountId = "101-001-26439625-001";
static const std::string accessToken = "19857f7ee685badb35b5db41af7f7435-834232d26024cd2d66319d14abcff5fd";
static const std::string streamDomain = "https://stream-fxtrade.oanda.com";
static const std::string apiDomain = "https://api-fxpractice.oanda.com";


#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define MY_UTC_OFFSET  -21600 // - 6 hours

#define TAKE_PROFIT_PRC 0.02f
#define STOP_LOSS_PRC   0.01f
#define UNITS_TO_BUY    50000

#define BUY_LOG_FILE_PATH "buyLogs.txt"
#define MIN_PRICE_LOG_FILE_PATH "minPriceLog.txt"
#define INFO_LOG_FILE_PATH "pricesData/"

enum OrderTypes
{
	BUY, SELL
};

struct Trade
{
	int id;
	int type;
	std::string name;
	float value;
	bool profit;
};

struct Instrument
{
	std::string name;
	int displayPrecision;
	float currentPrice;

};

class Oanda
{
public:
	Oanda();
	~Oanda();

	void Launch();

	void Cycle();

	void RefreshInformation();

private:
	Instrument currentInstrument;
	Trade lastTrade;
	std::string currentInstrumentName;
	std::string candleTime;
	std::vector<Trade> currentTrades;
	std::vector<Trade> PrevTrades;
	Context::Ptr context;
	int lastTransactionId;
private:
	void WriteBuyLog(std::string log);
	void WriteMinPriceLog(std::string log);
	void WriteInfoLog();
	void LoadCurrentTrades();
	void LoadLastTransaction();
	void LoadInstrument();

	void PlaceOrder(int type, Trade& item, std::string name, int units);
	void SetTakeProfit(int tradeId, std::string price);
	void SetStopLoss(int tradeId, std::string price);


	void LoadCurrentPrice(Instrument& item, int type);
	void LoadCandles(Instrument& item);
	void SetupReqHeader(HTTPRequest& req);

	float GetStopLossPrice(float price, int type);
	float GetTakeProfitPrice(float price, int type);

	int GetUNIXTime(bool withUTCOffset = false);
	std::string ConvertUNIXTime(time_t time, bool onlyDate = false);
	std::string GetFloatFormatStr(std::string str, int num);
};


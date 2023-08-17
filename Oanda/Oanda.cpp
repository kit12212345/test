#include "Oanda.h"
#include "Functions.hpp"
#include <thread>

Oanda::Oanda()
{
    currentInstrumentName = "NZD/USD";
    context = new Context(Context::CLIENT_USE, "", "", "", Context::VERIFY_NONE, 9, false, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
}

Oanda::~Oanda()
{
}

void Oanda::Launch()
{
    LoadInstrument();
    LoadCandles(currentInstrument);
}


void Oanda::Cycle()
{
    LoadCurrentTrades();
    if (currentTrades.size() == 0)
    {
        lastTrade.name = currentInstrument.name;
        LoadLastTransaction();


        int orderType = BUY;

        if (lastTrade.profit)
        {
            orderType = lastTrade.type;
        }
        else
        {
            orderType = lastTrade.type == SELL ? BUY : SELL;
        }

        int units = lastTrade.value * 1.2f;

        LoadCurrentPrice(currentInstrument, orderType);

        Trade curTrade;
        PlaceOrder(orderType, curTrade, currentInstrument.name, units);

        std::string takeProfitPrice = GetFloatFormatStr(std::to_string(GetTakeProfitPrice(currentInstrument.currentPrice, orderType)), currentInstrument.displayPrecision);
        std::string stopLossPrice = GetFloatFormatStr(std::to_string(GetStopLossPrice(currentInstrument.currentPrice, orderType)), currentInstrument.displayPrecision);

        SetTakeProfit(curTrade.id, takeProfitPrice);
        SetStopLoss(curTrade.id, stopLossPrice);


    }
}

void Oanda::RefreshInformation()
{
    LoadCurrentTrades();
}

void Oanda::WriteBuyLog(std::string log)
{
    std::ofstream logFile;
    logFile.open(BUY_LOG_FILE_PATH, std::ios_base::app);
    logFile << log;
    logFile.close();
}

void Oanda::WriteMinPriceLog(std::string log)
{
    std::ofstream logFile;
    logFile.open(MIN_PRICE_LOG_FILE_PATH, std::ios_base::app);
    logFile << log;
    logFile.close();
}

std::string Oanda::GetFloatFormatStr(std::string str, int num)
{
    size_t foundPt = str.find(".");
    if (foundPt == std::string::npos) str += ".00";
    else
    {
        std::string tempStr = str.substr(foundPt, str.length() - 1);
        if (tempStr.length() > 2)
        {
            str = str.substr(0, foundPt + num);
        }
    }

    return str;
}

void Oanda::WriteInfoLog()
{
    std::string logPath = INFO_LOG_FILE_PATH + ConvertUNIXTime(GetUNIXTime(true), true) + "_infoLog.txt";

    std::ofstream logFile;
    logFile.open(logPath, std::ios_base::app);

    std::string log;
    log += "\n\n~~~~~~~~~~~ " + ConvertUNIXTime(GetUNIXTime(true)) + " ~~~~~~~~~~~\n\n";
    log += "Name    Grow prc min: 30, 60, 90, 120\n";

    logFile << log;
    logFile.close();
}

void Oanda::PlaceOrder(int type, Trade& item, std::string name, int units)
{
    if (type == SELL) units *= -1.0f;

    std::string body = "{";

    body += "\"order\": {";
    body += "\"units\": \"" + std::to_string(units) + "\",";
    body += "\"instrument\": \"" + name + "\",";
    body += "\"timeInForce\": \"FOK\",";
    body += "\"type\": \"MARKET\",";
    body += "\"positionFill\": \"DEFAULT\"";
    body += "}";
    body += "}";


    URI uri(apiDomain + "/v3/accounts/" + accountId + "/orders");

    HTTPSClientSession session(uri.getHost(), uri.getPort(), context);
    session.setKeepAlive(true);

    string path(uri.getPathAndQuery());
    if (path.empty()) path = "/";

    HTTPRequest req(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
    req.setContentLength(body.length());

    SetupReqHeader(req);

    std::ostream& myOStream = session.sendRequest(req);
    myOStream << body;

    HTTPResponse res;
    istream& rs = session.receiveResponse(res);

    boost::property_tree::ptree pt;

    boost::property_tree::read_json(rs, pt);

    boost::property_tree::ptree node = pt.get_child("orderFillTransaction");
    boost::property_tree::ptree nodeTradeOpened = node.get_child("tradeOpened");

    item.id = std::stoi(nodeTradeOpened.get<std::string>("tradeID"));
    item.name = name;
}

void Oanda::SetTakeProfit(int tradeId, std::string price)
{
    std::string body = "{";

    body += "\"order\": {";
    body += "\"timeInForce\": \"GTC\",";
    body += "\"price\": \"" + price + "\",";
    body += "\"type\": \"TAKE_PROFIT\",";
    body += "\"tradeID\": \"" + std::to_string(tradeId) + "\"";
    body += "}";
    body += "}";


    URI uri(apiDomain + "/v3/accounts/" + accountId + "/orders");

    HTTPSClientSession session(uri.getHost(), uri.getPort(), context);
    session.setKeepAlive(true);

    string path(uri.getPathAndQuery());
    if (path.empty()) path = "/";

    HTTPRequest req(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
    req.setContentLength(body.length());

    SetupReqHeader(req);

    std::ostream& myOStream = session.sendRequest(req);
    myOStream << body;

    HTTPResponse res;
    istream& rs = session.receiveResponse(res);
}

void Oanda::SetStopLoss(int tradeId, std::string price)
{
    std::string body = "{";

    body += "\"order\": {";
    body += "\"timeInForce\": \"GTC\",";
    body += "\"price\": \"" + price + "\",";
    body += "\"type\": \"STOP_LOSS\",";
    body += "\"tradeID\": \"" + std::to_string(tradeId) + "\"";
    body += "}";
    body += "}";


    URI uri(apiDomain + "/v3/accounts/" + accountId + "/orders");

    HTTPSClientSession session(uri.getHost(), uri.getPort(), context);
    session.setKeepAlive(true);

    string path(uri.getPathAndQuery());
    if (path.empty()) path = "/";

    HTTPRequest req(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
    req.setContentLength(body.length());

    SetupReqHeader(req);

    std::ostream& myOStream = session.sendRequest(req);
    myOStream << body;

    HTTPResponse res;
    istream& rs = session.receiveResponse(res);
}


void Oanda::SetupReqHeader(HTTPRequest& req)
{
    req.set("Authorization", std::string("Bearer ") + accessToken);
    req.set("Content-Type", std::string("application/json"));
}

void Oanda::LoadCurrentTrades()
{
    currentTrades.clear();

    URI uri(apiDomain + std::string("/v3/accounts/") + accountId + std::string("/trades"));

    HTTPSClientSession session(uri.getHost(), uri.getPort(), context);
    session.setKeepAlive(true);

    // prepare path
    string path(uri.getPathAndQuery());
    if (path.empty()) path = "/";

    // send request
    HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
    SetupReqHeader(req);
    session.sendRequest(req);

    // get response
    HTTPResponse res;
    istream& rs = session.receiveResponse(res);


    boost::property_tree::ptree pt;

    boost::property_tree::read_json(rs, pt);

    boost::property_tree::ptree node = pt.get_child("trades");

    for (boost::property_tree::ptree::iterator it = node.begin(); it != node.end(); ++it)
    {
        Trade trade;

        int id = std::stoi(it->second.get<std::string>("id"));
        std::string instName = it->second.get<std::string>("instrument");

        trade.id = id;
        trade.name = instName;

        currentTrades.push_back(trade);
    }

    if (currentTrades.size() >= 1)
    {
        lastTrade = currentTrades[currentTrades.size() - 1];
    }
    else
    {
        lastTransactionId = std::stoi(pt.get<std::string>("lastTransactionID"));
    }
}

void Oanda::LoadLastTransaction()
{
    std::string t = apiDomain + std::string("/v3/accounts/") + accountId + std::string("/transactions/") + std::to_string(lastTransactionId);

    URI uri(t);

    HTTPSClientSession session(uri.getHost(), uri.getPort(), context);
    session.setKeepAlive(true);

    // prepare path
    string path(uri.getPathAndQuery());
    if (path.empty()) path = "/";

    // send request
    HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
    SetupReqHeader(req);
    session.sendRequest(req);

    // get response
    HTTPResponse res;
    istream& rs = session.receiveResponse(res);


    boost::property_tree::ptree pt;

    boost::property_tree::read_json(rs, pt);

    boost::property_tree::ptree node = pt.get_child("transaction");

    if (node.get<std::string>("type") != "ORDER_FILL")
    {
        lastTransactionId--;
        LoadLastTransaction();
        return;
    }

    lastTrade.profit = std::stof(node.get<std::string>("pl")) >= 0;
    lastTrade.type = std::stoi(node.get<std::string>("units")) > 0 ? BUY : SELL;
    lastTrade.value = std::abs(std::stoi(node.get<std::string>("units")));

}

void Oanda::LoadInstrument()
{
    try
    {
        URI uri(apiDomain + std::string("/v3/accounts/") + accountId + std::string("/instruments?instruments=" + currentInstrumentName));

        HTTPSClientSession session(uri.getHost(), uri.getPort(), context);
        session.setKeepAlive(true);

        // prepare path
        string path(uri.getPathAndQuery());
        if (path.empty()) path = "/";

        // send request
        HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
        SetupReqHeader(req);
        session.sendRequest(req);

        // get response
        HTTPResponse res;
        istream& rs = session.receiveResponse(res);


        boost::property_tree::ptree pt;

        boost::property_tree::read_json(rs, pt);

        boost::property_tree::ptree node = pt.get_child("instruments");

        for (boost::property_tree::ptree::iterator it = node.begin(); it != node.end(); ++it)
        {
            std::string instName = it->second.get<std::string>("name");
            currentInstrument.name = instName;
            currentInstrument.displayPrecision = stoi(it->second.get<std::string>("displayPrecision"));
        }
    }
    catch (const Exception& err)
    {
        std::cout << err.what() << std::endl;
    }
    catch (const std::exception& err)
    {
        std::cout << err.what() << std::endl;
    }


}


void Oanda::LoadCurrentPrice(Instrument& item, int type)
{
    std::string t = apiDomain + "/v3/accounts/" + accountId + "/pricing?instruments=" + item.name;

    URI uri(apiDomain + "/v3/accounts/" + accountId + "/pricing?instruments=" + item.name);

    HTTPSClientSession session(uri.getHost(), uri.getPort(), context);
    session.setKeepAlive(true);

    // prepare path
    string path(uri.getPathAndQuery());
    if (path.empty()) path = "/";

    // send request
    HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
    SetupReqHeader(req);
    session.sendRequest(req);

    // get response
    HTTPResponse res;
    istream& rs = session.receiveResponse(res);


    boost::property_tree::ptree pt;

    boost::property_tree::read_json(rs, pt);

    boost::property_tree::ptree node = pt.get_child("prices");
    for (boost::property_tree::ptree::iterator it = node.begin(); it != node.end(); ++it)
    {
        std::string key = type == BUY ? "closeoutAsk" : "closeoutBid";
        item.currentPrice = std::stof(it->second.get<std::string>(key));
    }


   
}

void Oanda::LoadCandles(Instrument& item)
{
    std::string url = apiDomain + "/v3/instruments/" + currentInstrument.name + "/candles?count=1&price=M&granularity=S30";
    URI uri(url);

    HTTPSClientSession session(uri.getHost(), uri.getPort(), context);
    session.setKeepAlive(true);

    // prepare path
    string path(uri.getPathAndQuery());
    if (path.empty()) path = "/";

    // send request
    HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
    SetupReqHeader(req);
    session.sendRequest(req);

    // get response
    HTTPResponse res;
    istream& rs = session.receiveResponse(res);


    boost::property_tree::ptree pt;

    boost::property_tree::read_json(rs, pt);

    boost::property_tree::ptree node = pt.get_child("candles");
    for (boost::property_tree::ptree::iterator it = node.begin(); it != node.end(); ++it)
    {
        bool complite = it->second.get<std::string>("complete") == "true";
    }
    int a = 0;
}

float Oanda::GetStopLossPrice(float price, int type)
{
    float v = ((price / 100.0f) * STOP_LOSS_PRC);
    return type == SELL ? price + v : price - v;
}
float Oanda::GetTakeProfitPrice(float price, int type)
{
    float v = ((price / 100.0f) * TAKE_PROFIT_PRC);
    return type == SELL ? price - v : price + v;
}

int Oanda::GetUNIXTime(bool withUTCOffset)
{
    const auto p1 = std::chrono::system_clock::now();
    int unixTime = (int)std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
    return unixTime;
}

std::string Oanda::ConvertUNIXTime(time_t time, bool onlyDate)
{
    char time_buf[80];
    struct tm ts;
    localtime_s(&ts, &time);
    if (onlyDate)
    {
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d", &ts);
    }
    else
    {
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &ts);
    }
    return std::string(time_buf);
}


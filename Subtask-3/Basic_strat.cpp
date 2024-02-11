#include <bits/stdc++.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
using namespace std;

struct StockData {
    string date;
    double open;
    double close;
    double high;
    double low;
    double ltp;
    double volume;
    double value;
    double prevClose;
    int numTrades;
};

string decreaseDays(const string& dateString, int n) {
    // Parse the input string to obtain day, month, and year
    tm timeinfo = {};
    istringstream ss(dateString);
    ss >> get_time(&timeinfo, "%d/%m/%Y");

    if (ss.fail()) {
        cerr << "Error: Failed to parse date string\n";
        return "";
    }

    // Convert tm to chrono::system_clock::time_point
    chrono::system_clock::time_point currentTimePoint = chrono::system_clock::from_time_t(mktime(&timeinfo));

    // Subtract n days
    chrono::hours hoursPerDay(24);
    chrono::system_clock::time_point newTimePoint = currentTimePoint - chrono::duration_cast<chrono::hours>(hoursPerDay * n);

    // Convert hrono::system_clock::time_point to time_t
    time_t newTime = chrono::system_clock::to_time_t(newTimePoint);

    // Convert time_t to struct tm
    struct tm* newTimeStruct = localtime(&newTime);
    // Convert struct tm to string
    ostringstream result;
    result << put_time(newTimeStruct, "%d/%m/%Y");

    return result.str();
}

// Helper function to convert date format
string convertDateFormat(const string& originalDate) {
    // Assuming the original format is "dd-mm-yyyy"
    // Convert it to "dd/mm/yyyy"
    string convertedDate = originalDate;
    size_t dashPos = convertedDate.find('-');
    while (dashPos != string::npos) {
        convertedDate.replace(dashPos, 1, "/");
        dashPos = convertedDate.find('-', dashPos + 1);
    }
    return convertedDate;
}

void writePnL(double pnl, int position, double lastDayPrice){
    ofstream finalPnLFile("final_pnl.txt");
    if (!finalPnLFile.is_open()) {
        std::cerr << "Error: Failed to open file for writing\n";
        return;
    }
    double final_pnl = pnl + position*lastDayPrice;
    finalPnLFile<<final_pnl<<endl;
    finalPnLFile.close();
}

vector<StockData> getStockData(string symbol, string start_date, string end_date, int n) {

    string command = "python3 main.py " + symbol + " " + to_string(n) + " " + start_date + " " + end_date;
    system(command.c_str());

    ifstream csvFile(symbol+".csv");

    if (!csvFile.is_open()) {
        cerr << "Error: Failed to open CSV file\n";
        exit(1);
    }

    vector<StockData> stockData;

    string line;
    getline(csvFile, line); // Skip the first line

    // Read each line from the CSV file
    while (getline(csvFile, line)) {
        istringstream iss(line);
        string token;

        // Create a StockData object for each line
        StockData data;

        // Parse each token in the line
        getline(iss, token, ',');
        data.date = convertDateFormat(token); // Convert date format
        getline(iss, token, ',');
        data.open = stod(token);
        getline(iss, token, ',');
        data.close = stod(token);
        getline(iss, token, ',');
        data.high = stod(token);
        getline(iss, token, ',');
        data.low = stod(token);
        getline(iss, token, ',');
        data.ltp = stod(token);
        getline(iss, token, ',');
        data.volume = stod(token);
        getline(iss, token, ',');
        data.value = stod(token);
        getline(iss, token, ',');
        data.prevClose = stod(token);
        getline(iss, token, ',');
        data.numTrades = stoi(token);
        stockData.push_back(data);
    }

    return stockData;
}

void BasicStrategy(string symbol, string start_date, string end_date, int n, int x) {

    // Get stock data
    vector<StockData> stockData = getStockData(symbol, start_date, end_date, n);

    int position = 0;
    int dir_count = 0;
    double pnl = 0.0;

    // Open files for writing
    ofstream dailyCashflowFile("daily_pnl.csv");
    ofstream orderStatisticsFile("order_statistics.csv");

    // Check if files are opened successfully
    if (!dailyCashflowFile.is_open() || !orderStatisticsFile.is_open()) {
        std::cerr << "Error: Failed to open files for writing\n";
        return;
    }

    // Write headers to CSV files
    dailyCashflowFile << "Date,Cashflow\n";
    orderStatisticsFile << "Date,Order_dir,Quantity,Price\n";


    for(int i = 1; i < stockData.size(); i++){
        if(i<n){
            dir_count += (stockData[i].close - stockData[i-1].close > 0 ? 1 : -1);
        }
        else{
            dir_count += (stockData[i].close - stockData[i-1].close > 0 ? 1 : -1);
            if(dir_count ==n && position <x){
                orderStatisticsFile<<stockData[i].date<<",BUY,1,"<<stockData[i].close<<endl;
                pnl -= stockData[i].close;
                position++;
            }
            else if(dir_count == -n && position > -x){
                orderStatisticsFile<<stockData[i].date<<",SELL,1,"<<stockData[i].close<<endl;
                pnl += stockData[i].close;
                position--;
            }
            dir_count -= (stockData[i-n+1].close - stockData[i-n].close > 0 ? 1 : -1);
            dailyCashflowFile<<stockData[i].date<<","<<pnl<<endl;
        }
    }
    dailyCashflowFile.close();
    orderStatisticsFile.close();

    writePnL(pnl, position, stockData[stockData.size()-1].close);
    
}

void DMAStrategy(string symbol, string start_date, string end_date, int n, int x, int p) {

    // Get stock data
    vector<StockData> stockData = getStockData(symbol, start_date, end_date,n-1);

    int position = 0;
    double price_sum = 0.0;
    double square_price_sum = 0.0;
    double pnl = 0.0;

    // Open files for writing
    ofstream dailyCashflowFile("daily_pnl.csv");
    ofstream orderStatisticsFile("order_statistics.csv");

    // Check if files are opened successfully
    if (!dailyCashflowFile.is_open() || !orderStatisticsFile.is_open()) {
        std::cerr << "Error: Failed to open files for writing\n";
        return;
    }

    // Write headers to CSV files
    dailyCashflowFile << "Date,Cashflow\n";
    orderStatisticsFile << "Date,Order_dir,Quantity,Price\n";


    for(int i = 0; i < stockData.size(); i++){
        if(i<n-1){
            price_sum += stockData[i].close;
            square_price_sum += stockData[i].close*stockData[i].close;
        }
        else{
            price_sum += stockData[i].close;
            square_price_sum += stockData[i].close*stockData[i].close;
            double mean = price_sum/n;
            double variance = (square_price_sum - (price_sum*price_sum)/n)/n;
            double std_dev = sqrt(variance);
            if( stockData[i].close >= mean + p*std_dev && position <x){
                orderStatisticsFile<<stockData[i].date<<",BUY,1,"<<stockData[i].close<<endl;
                pnl -= stockData[i].close;
                position++;
            }
            else if( stockData[i].close <= mean - p*std_dev && position > -x){
                orderStatisticsFile<<stockData[i].date<<",SELL,1,"<<stockData[i].close<<endl;
                pnl += stockData[i].close;
                position--;
            }
            price_sum -= stockData[i-n+1].close;
            square_price_sum -= stockData[i-n+1].close*stockData[i-n+1].close;
            dailyCashflowFile<<stockData[i].date<<","<<pnl<<endl;
        }
    }
    
    dailyCashflowFile.close();
    orderStatisticsFile.close();

    writePnL(pnl, position, stockData[stockData.size()-1].close);

}

void AMAStrategy(string symbol, string start_date, string end_date, int n, int x, int p, int max_hold_days, double c1, double c2) {

    // Get stock data
    vector<StockData> stockData = getStockData(symbol, start_date, end_date,n);

    int position = 0;
    double volatility = 0.0;
    double SF = 0.5;
    double AMA = 0.0;
    double pnl = 0.0;
    queue<int> buy_queue;
    queue<int> sell_queue;

    // Open files for writing
    ofstream dailyCashflowFile("daily_pnl.csv");
    ofstream orderStatisticsFile("order_statistics.csv");

    // Check if files are opened successfully
    if (!dailyCashflowFile.is_open() || !orderStatisticsFile.is_open()) {
        std::cerr << "Error: Failed to open files for writing\n";
        return;
    }

    // Write headers to CSV files
    dailyCashflowFile << "Date,Cashflow\n";
    orderStatisticsFile << "Date,Order_dir,Quantity,Price\n";

    for(int i =1; i<stockData.size(); i++){
        if(i<n){
            volatility += abs(stockData[i].close - stockData[i-1].close);
        }
        else{
            volatility += abs(stockData[i].close - stockData[i-1].close);
            double direction = abs(stockData[i].close - stockData[i-n].close);
            double ER = 0;
            int hold_flag = 0;
            int AMA_flag = 0;

            if(volatility!=0){
                ER = direction/volatility;
            }
            else{
                if(!buy_queue.empty() && i-buy_queue.front()>max_hold_days){ 
                    orderStatisticsFile<<stockData[i].date<<",SELL,1,"<<","<<stockData[i].close<<endl;
                    pnl+=stockData[i].close;
                    position--;
                    buy_queue.pop();
                }
                if(!sell_queue.empty() && i-sell_queue.front()>max_hold_days){
                    orderStatisticsFile<<stockData[i].date<<",BUY,1,"<<","<<stockData[i].close<<endl;
                    pnl-=stockData[i].close;
                    position++;
                    sell_queue.pop();
                }

                continue;
            }
            if(i>n){
                double c = (2*ER)/(1+c2);
                SF = SF*(1-c1) + ((c1*(c-1))/(c+1));
            }

            if(i==n){
                AMA = stockData[i].close;
            }
            else{
                AMA = AMA+ SF*(stockData[i].close - AMA);
            }

            if(stockData[i].close > AMA*(1+(p/100)) && position < x){
                AMA_flag = 1;
            }
            else if(stockData[i].close < AMA*(1-(p/100)) && position > -x){
                AMA_flag = -1;
            }
            if(!buy_queue.empty() && i-buy_queue.front()>max_hold_days){ 
                hold_flag = -1;
            }
            if(!sell_queue.empty() && i-sell_queue.front()>max_hold_days){
                hold_flag = 1;
            }
            
            if(AMA_flag != 0 || hold_flag != 0){
                if(AMA_flag == 0){
                    if(hold_flag == 1){
                        orderStatisticsFile<<stockData[i].date<<",BUY,1,"<<","<<stockData[i].close<<endl;
                        pnl-=stockData[i].close;
                        position++;
                        sell_queue.pop();
                    }
                    else if(hold_flag == -1){
                        orderStatisticsFile<<stockData[i].date<<",SELL,1,"<<","<<stockData[i].close<<endl;
                        pnl+=stockData[i].close;
                        position--;
                        buy_queue.pop();
                    }
                }
                else if(hold_flag==0){
                    if(AMA_flag == 1){
                        orderStatisticsFile<<stockData[i].date<<",BUY,1,"<<","<<stockData[i].close<<endl;
                        pnl-=stockData[i].close;
                        position++;
                        if(!sell_queue.empty()){
                            sell_queue.pop();
                        }
                        else{
                            buy_queue.push(i);
                        }
                    }
                    else if(AMA_flag == -1){
                        orderStatisticsFile<<stockData[i].date<<",SELL,1,"<<","<<stockData[i].close<<endl;
                        pnl+=stockData[i].close;
                        position--;
                        if(!buy_queue.empty()){
                            buy_queue.pop();
                        }
                        else{
                            sell_queue.push(i);
                        }
                    }
                }
                else if(hold_flag == AMA_flag){
                    if(hold_flag == 1){
                        orderStatisticsFile<<stockData[i].date<<",BUY,1,"<<","<<stockData[i].close<<endl;
                        pnl-=stockData[i].close;
                        position++;
                        sell_queue.pop();
                    }
                    else if(hold_flag == -1){
                        orderStatisticsFile<<stockData[i].date<<",SELL,1,"<<","<<stockData[i].close<<endl;
                        pnl+=stockData[i].close;
                        position--;
                        buy_queue.pop();
                    }
                }
                else if(hold_flag != AMA_flag){
                    if(hold_flag == 1){
                        sell_queue.pop();
                        sell_queue.push(i);
                    }
                    else if(hold_flag == -1){
                        buy_queue.pop();
                        buy_queue.push(i);
                    }
                }
            }
            dailyCashflowFile<<stockData[i].date<<","<<pnl<<endl;
            volatility -= abs(stockData[i-n+1].close - stockData[i-n].close);
        }
    }

    dailyCashflowFile.close();
    orderStatisticsFile.close();

    writePnL(pnl, position, stockData[stockData.size()-1].close);
}


void MACDStrategy(string symbol, string start_date, string end_date, int x){

    // Get stock data
    vector<StockData> stockData = getStockData(symbol, start_date, end_date, 0);

    int position = 0;
    double short_EWM = 0.0;
    double long_EWM = 0.0;
    double MACD = 0.0;
    double signal = 0.0;
    double pnl = 0.0;

    // Open files for writing
    ofstream dailyCashflowFile("daily_pnl.csv");
    ofstream orderStatisticsFile("order_statistics.csv");

    // Check if files are opened successfully
    if (!dailyCashflowFile.is_open() || !orderStatisticsFile.is_open()) {
        std::cerr << "Error: Failed to open files for writing\n";
        return;
    }

    // Write headers to CSV files
    dailyCashflowFile << "Date,Cashflow\n";
    orderStatisticsFile << "Date,Order_dir,Quantity,Price\n";


    for(int i = 0; i < stockData.size(); i++){
        if(i==0){
            short_EWM = stockData[i].close;
            long_EWM = stockData[i].close;
        }
        else{
            short_EWM = ((2*(stockData[i].close - short_EWM))/(13)) + short_EWM;
            long_EWM = ((2*(stockData[i].close - long_EWM))/(27)) + long_EWM;
            MACD = short_EWM - long_EWM;
            signal = ((2*(MACD - signal))/(10)) + signal;
            if(MACD > signal && position < x){
                orderStatisticsFile<<stockData[i].date<<",BUY,1,"<<stockData[i].close<<endl;
                pnl -= stockData[i].close;
                position++;
            }
            else if(MACD < signal && position > -x){
                orderStatisticsFile<<stockData[i].date<<",SELL,1,"<<stockData[i].close<<endl;
                pnl += stockData[i].close;
                position--;
            }
        }
        dailyCashflowFile<<stockData[i].date<<","<<pnl<<endl;
    }
    
    dailyCashflowFile.close();
    orderStatisticsFile.close();

    writePnL(pnl, position, stockData[stockData.size()-1].close);
}

void RSIStrategy(string symbol, string start_date, string end_date, int n, int x, double oversold_threshold, double overbought_threshold){
    // Get stock data
    vector<StockData> stockData = getStockData(symbol, start_date, end_date,n);

    int position = 0;
    double gain_sum = 0.0;
    double loss_sum = 0.0;
    double avg_gain = 0.0;
    double avg_loss = 0.0;
    double pnl = 0.0;

    // Open files for writing
    ofstream dailyCashflowFile("daily_pnl.csv");
    ofstream orderStatisticsFile("order_statistics.csv");

    // Check if files are opened successfully
    if (!dailyCashflowFile.is_open() || !orderStatisticsFile.is_open()) {
        std::cerr << "Error: Failed to open files for writing\n";
        return;
    }

    // Write headers to CSV files
    dailyCashflowFile << "Date,Cashflow\n";
    orderStatisticsFile << "Date,Order_dir,Quantity,Price\n";


    for(int i = 1; i < stockData.size(); i++){
        if(i<n){
            gain_sum += max(stockData[i].close-stockData[i-1].close, 0.0);
            loss_sum -= min(stockData[i].close-stockData[i-1].close, 0.0);
        }
        else{
            gain_sum += max(stockData[i].close-stockData[i-1].close, 0.0);
            loss_sum -= min(stockData[i].close-stockData[i-1].close, 0.0);
            avg_gain = gain_sum/n;
            avg_loss = loss_sum/n;
            double RS = avg_gain/avg_loss;
            double RSI = 100 - (100/(1+RS));

            if( RSI < oversold_threshold && position <x){
                orderStatisticsFile<<stockData[i].date<<",BUY,1,"<<stockData[i].close<<endl;
                pnl -= stockData[i].close;
                position++;
            }
            else if( RSI > overbought_threshold && position > -x){
                orderStatisticsFile<<stockData[i].date<<",SELL,1,"<<stockData[i].close<<endl;
                pnl += stockData[i].close;
                position--;
            }
            gain_sum -= max(stockData[i-n+1].close-stockData[i-n].close, 0.0);
            loss_sum += min(stockData[i-n+1].close-stockData[i-n].close, 0.0);
            dailyCashflowFile<<stockData[i].date<<","<<pnl<<endl;
        }
    }
    
    dailyCashflowFile.close();
    orderStatisticsFile.close();

    writePnL(pnl, position, stockData[stockData.size()-1].close);

}

void ADXStrategy(string symbol, string start_date, string end_date, int n, int x, double adx_threshold){

    // Get stock data
    vector<StockData> stockData = getStockData(symbol, start_date, end_date,2);

    int position = 0;
    double TR = 0.0;
    double DM_plus = 0.0;
    double DM_minus = 0.0;
    double ATR = 0.0;
    double DI_plus = 0.0;
    double DI_minus = 0.0;
    double DX = 0.0;
    double ADX = 0.0;
    double pnl = 0.0;

    // Open files for writing
    ofstream dailyCashflowFile("daily_pnl.csv");
    ofstream orderStatisticsFile("order_statistics.csv");

    // Check if files are opened successfully
    if (!dailyCashflowFile.is_open() || !orderStatisticsFile.is_open()) {
        cerr << "Error: Failed to open files for writing\n";
        return;
    }

    // Write headers to CSV files
    dailyCashflowFile << "Date,Cashflow\n";
    orderStatisticsFile << "Date,Order_dir,Quantity,Price\n";

    for(int i =2; i<stockData.size(); i++){
        TR = max(stockData[i].high - stockData[i].low, max(stockData[i].high - stockData[i-1].close, stockData[i-1].close - stockData[i].low));
        DM_plus = max(0.0, stockData[i].high - stockData[i-1].high);
        DM_minus = max(0.0, stockData[i].low - stockData[i-1].low);
        if(i==2){
            ATR = TR;
            DI_plus = DM_plus/ATR;
            DI_minus = DM_minus/ATR;
            double num = (DI_plus - DI_minus);
            double den = DI_plus + DI_minus;
            if(den==0){
                DX = 0;
            }
            else{
                DX = (num/den)*100;
            }
            ADX = DX;
        }
        else{
            ATR = ((2*(TR - ATR))/(n+1)) + ATR;
            DI_plus = ((2*((DM_plus/ATR) - DI_plus))/(n+1)) + DI_plus;
            DI_minus = ((2*((DM_minus/ATR) - DI_minus))/(n+1)) + DI_minus;
            double num = (DI_plus - DI_minus);
            double den = DI_plus + DI_minus;
            if(den==0){
                DX = 0;
            }
            else{
                DX = (num/den)*100;
            }
            ADX = ((2*(DX - ADX))/(n+1)) + ADX;
        }

        if(ADX > adx_threshold && position <x){
            orderStatisticsFile<<stockData[i].date<<",BUY,1,"<<stockData[i].close<<endl;
            pnl -= stockData[i].close;
            position++;
        }
        else if(ADX < adx_threshold && position > -x){
            orderStatisticsFile<<stockData[i].date<<",SELL,1,"<<stockData[i].close<<endl;
            pnl += stockData[i].close;
            position--;
        }
        dailyCashflowFile<<stockData[i].date<<","<<pnl<<endl;
    }

    dailyCashflowFile.close();
    orderStatisticsFile.close();

    writePnL(pnl, position, stockData[stockData.size()-1].close);
}


int main(int argc, char *argv[]) {
    if (argc < 6) {
        cerr << "Usage: " << argv[0] << " strategy symbol x start_date end_date\n";
        return 1;
    }

    // Extract command line arguments
    string strategy = argv[1];
    string symbol = argv[2];
    int x = stoi(argv[3]);
    string start_date = argv[4];
    string end_date = argv[5];

    if (strategy == "BASIC") {
        int n = stoi(argv[6]);
        BasicStrategy(symbol, start_date, end_date, n, x);
    }else if(strategy == "DMA"){
        int n = stoi(argv[6]);
        int p = stoi(argv[7]); 
        DMAStrategy(symbol, start_date, end_date, n, x, p);
    }
    else if(strategy == "DMA++"){
        int n = stoi(argv[6]);
        int p = stoi(argv[7]);
        int max_hold_days = stoi(argv[8]);
        double c1 = stod(argv[9]);
        double c2 = stod(argv[10]);
        AMAStrategy(symbol, start_date, end_date, n, x, p, max_hold_days, c1, c2);
    }
    else if(strategy == "MACD"){
        MACDStrategy(symbol, start_date, end_date, x);
    }
    else if(strategy == "RSI"){
        int n = stoi(argv[6]);
        double oversold_threshold = stod(argv[7]);
        double overbought_threshold = stod(argv[8]);
        RSIStrategy(symbol, start_date, end_date, n, x, oversold_threshold, overbought_threshold);
    }
    else if(strategy == "ADX"){
        int n = stoi(argv[6]);
        double adx_threshold = stod(argv[7]);
        ADXStrategy(symbol, start_date, end_date, n, x, adx_threshold);
    }
    else{
        cerr << "Error: Invalid strategy\n";
        return 1;
    }

    return 0;
}

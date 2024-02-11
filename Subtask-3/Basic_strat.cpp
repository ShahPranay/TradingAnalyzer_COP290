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

vector<StockData> getStockData(string symbol, string start_date, string end_date) {

    string command = "python3 main.py " + symbol + " " + start_date + " " + end_date;
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
        data.numTrades = stoi(token);
        stockData.push_back(data);
    }

    return stockData;
}

void BasicStrategy(string symbol, string start_date, string end_date, int n, int x) {
    start_date = decreaseDays(start_date, n);

    // Get stock data
    vector<StockData> stockData = getStockData(symbol, start_date, end_date);
    reverse(stockData.begin(), stockData.end());

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

    ofstream finalPnLFile("final_pnl.txt");
    if (!finalPnLFile.is_open()) {
        std::cerr << "Error: Failed to open file for writing\n";
        return;
    }
    double final_pnl = pnl + position*stockData[stockData.size()-1].close;
    finalPnLFile<<final_pnl<<endl;
    finalPnLFile.close();
}

void DMAStrategy(string symbol, string start_date, string end_date, int n, int x, double p) {
    start_date = decreaseDays(start_date, n);

    // Get stock data
    vector<StockData> stockData = getStockData(symbol, start_date, end_date);
    reverse(stockData.begin(), stockData.end());

    int position = 0;
    double price_sum = 0;
    double square_price_sum = 0;
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
        if(i<n){
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
            price_sum -= stockData[i-n].close;
            square_price_sum -= stockData[i-n].close*stockData[i-n].close;
            dailyCashflowFile<<stockData[i].date<<","<<pnl<<endl;
        }
    }
    
    dailyCashflowFile.close();
    orderStatisticsFile.close();

    ofstream finalPnLFile("final_pnl.txt");
    if (!finalPnLFile.is_open()) {
        std::cerr << "Error: Failed to open file for writing\n";
        return;
    }
    double final_pnl = pnl + position*stockData[stockData.size()-1].close;
    finalPnLFile<<final_pnl<<endl;
    finalPnLFile.close();

}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        cerr << "Usage: " << argv[0] << " strategy symbol n x start_date end_date\n";
        return 1;
    }

    // Extract command line arguments
    string strategy = argv[1];
    string symbol = argv[2];
    int n = stoi(argv[3]);
    int x = stoi(argv[4]);
    string start_date = argv[5];
    string end_date = argv[6];

    if (strategy == "BASIC") {
        BasicStrategy(symbol, start_date, end_date, n, x);
    }else if(strategy == "DMA"){
        DMAStrategy(symbol, start_date, end_date, n, x, 0.1);
    }
    else{
        cerr << "Error: Invalid strategy\n";
        return 1;
    }

    return 0;
}

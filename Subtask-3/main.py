import sys
from datetime import datetime
from datetime import date
from jugaad_data.nse import stock_df
    

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python main.py SYMBOL start_date end_date")
        sys.exit(1)

    symbol = sys.argv[1]
    from_date = sys.argv[2]
    to_date = sys.argv[3]
    
    from_date = datetime.strptime(from_date, "%d/%m/%Y")
    to_date = datetime.strptime(to_date, "%d/%m/%Y")

    df = stock_df(symbol=symbol, from_date=from_date,
                to_date=to_date, series="EQ")
    required_columns = ['DATE', 'OPEN', 'CLOSE', 'HIGH', 'LOW', 'LTP', 'VOLUME', 'VALUE', 'NO OF TRADES']
    df = df[required_columns]

    # Save data from df to csv
    df.to_csv(symbol+".csv", index=False)

    # Save data from df to feather
    # df.to_feather(symbol+".feather")

    # Save data from df to pickle
    # df.to_pickle(symbol+".pickle")
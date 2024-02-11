import sys
from datetime import datetime, timedelta
from jugaad_data.nse import stock_df
    

if __name__ == "__main__":
    if len(sys.argv) != 5:
        print("Usage: python main.py SYMBOL n start_date end_date")
        sys.exit(1)

    symbol = sys.argv[1]
    n = int(sys.argv[2])
    from_date = sys.argv[3]
    to_date = sys.argv[4]
    from_date = datetime.strptime(from_date, "%d/%m/%Y")
    to_date = datetime.strptime(to_date, "%d/%m/%Y")

    x = n/2
    new_from_date = from_date - timedelta(days=n+x)
    df = stock_df(symbol=symbol, from_date=new_from_date,
                to_date=to_date, series="EQ")
    df = df.iloc[::-1]

    flag = True
    if(n==0):
        flag = False
        
    while flag:
        i = 0
        for index, row in df.iterrows():
            date_value = row['DATE']
            if date_value < from_date:
                i += 1
            else:
                break
        
        if i<n:
            if x==0:
                x+=1
            x*=2
            new_from_date = from_date - timedelta(days=n+x)
            df = stock_df(symbol=symbol, from_date=new_from_date,
                to_date=to_date, series="EQ")
            df = df.iloc[::-1]
        else:
            flag = False
            i-=n
            df = df.iloc[i:]
            
    required_columns = ['DATE', 'OPEN', 'CLOSE', 'HIGH', 'LOW', 'LTP', 'VOLUME', 'VALUE', 'PREV. CLOSE', 'NO OF TRADES' ]
    df = df[required_columns]
        
    # Save data from df to csv
    df.to_csv(symbol+".csv", index=False)
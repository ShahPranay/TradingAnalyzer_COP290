from flask import Flask, render_template, request, redirect, url_for, flash, session, jsonify
from flask_sqlalchemy import SQLAlchemy
from sqlalchemy.util import method_is_overridden
from werkzeug.security import generate_password_hash, check_password_hash
import yfinance as yf
import pandas as pd
import json

app = Flask(__name__)
app.secret_key = 'your_secret_key'  # Replace with your actual secret key
Tickers_df = pd.read_csv("tickers_data.csv", index_col=0)
Tickers_dict = Tickers_df.to_dict(orient='index')
Symbols_df = pd.read_csv('symbols.csv')
symbolsList = Symbols_df['Symbol'].tolist()
symbolsList.insert(0, '')

# Database Configuration
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///users.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db = SQLAlchemy(app)

# User Model
class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(100), unique=True, nullable=False)
    password_hash = db.Column(db.String(200), nullable=False)

# Initialize Database within Application Context
with app.app_context():
    db.create_all()

@app.route('/')
def index():
    return render_template('login.html')

@app.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        hashed_password = generate_password_hash(password, method='pbkdf2:sha256')

        new_user = User(username=username, password_hash=hashed_password)
        db.session.add(new_user)
        db.session.commit()

        flash('Registration successful! Please login.')
        return redirect(url_for('index'))

    return render_template('register.html')

@app.route('/login', methods=['POST'])
def login():
    username = request.form['username']
    password = request.form['password']
    user = User.query.filter_by(username=username).first()

    if user and check_password_hash(user.password_hash, password):
        session['user_id'] = user.id
        session['username'] = user.username
        return redirect(url_for('dashboard'))
    else:
        flash('Invalid username or password')
        return redirect(url_for('index'))

@app.route('/dashboard', methods=['GET'])
def dashboard():
    return render_template('dashboard.html', username=session['username'])

@app.route('/data', methods=['POST'])
def getdata():
    stock_name = request.json['stock_name']

    tkr = yf.Ticker(stock_name)
    stk_data = tkr.history(period='max', interval='1d')

    # Ensure data is sorted by time
    stk_data = stk_data.sort_index()
    print(stk_data.columns)
    # Formatting data for Chart.js financial chart
    ohlc_data = []
    for index, row in stk_data.iterrows():
        ohlc_data.append({
            'x': index.strftime('%Y-%m-%dT%H:%M:%S'),  # ISO 8601 format
            'o': row['Open'],
            'h': row['High'],
            'l': row['Low'],
            'c': row['Close'],
            'v': row['Volume']
        })

    chart_data = {
        'label': stock_name,
        'data': ohlc_data
    }
    print("reqest processed")
    return jsonify(chart_data)

@app.route('/stock_names', methods=['POST'])
def get_stock_names():
    # Fetch and return all available stock names
    # You need to implement this based on your data source (e.g., database)
    # For now, assume you have a list of stock names
    filter_type = request.json['filter_type']
    min_filter_value = float(request.json['min_filter_value'])
    max_filter_value = float(request.json['max_filter_value'])
    stock_names = symbolsList
    if filter_type == 'default' or (min_filter_value == 0.0 and max_filter_value == 0.0):
        pass
    elif filter_type == 'trailingPE':
        stock_names = [symbol for symbol, info in Tickers_dict.items() if
                    info.get('trailingPE', 0.0) > min_filter_value and
                    info.get('trailingPE', 0.0) < max_filter_value]
    elif filter_type == 'forwardPE':
        stock_names = [symbol for symbol, info in Tickers_dict.items() if
                    info.get('forwardPE', 0.0) > min_filter_value and
                    info.get('forwardPE', 0.0) < max_filter_value]
    elif filter_type == 'fiftyDayAverage':
        stock_names = [symbol for symbol, info in Tickers_dict.items() if
                    info.get('fiftyDayAverage', 0.0) > min_filter_value and
                    info.get('fiftyDayAverage', 0.0) < max_filter_value]
    elif filter_type == 'twoHundredDayAverage':
        stock_names = [symbol for symbol, info in Tickers_dict.items() if
                    info.get('twoHundredDayAverage', 0.0) > min_filter_value and
                    info.get('twoHundredDayAverage', 0.0) < max_filter_value]
    return jsonify({'stock_names': stock_names})

@app.route('/stock_info', methods=['POST'])
def get_stock_info():
    stock_name = request.json['stock_name']
    stock_info = yf.Ticker(stock_name).info
    print(stock_info)
    dayLow = stock_info.get('dayLow', '-')
    dayHigh = stock_info.get('dayHigh', '-')
    yearLow = stock_info.get('fiftyTwoWeekLow', '-')
    yearHigh = stock_info.get('fiftyTwoWeekHigh', '-')
    Open = stock_info.get('open', '-')
    previousClose = stock_info.get('previousClose', '-')
    volume = stock_info.get('volume', '-')
    currentPrice = stock_info.get('currentPrice', '-')
    marketCap = stock_info.get('marketCap', '-')
    if(marketCap != '-'):
        marketCap = toCr(marketCap)
    trailingPE = stock_info.get('trailingPE', '-')
    forwardPE = stock_info.get('forwardPE', '-')
    fiftyDayAverage = stock_info.get('fiftyDayAverage', '-')
    twoHundredDayAverage = stock_info.get('twoHundredDayAverage', '-')
    dividendYield = stock_info.get('dividendYield', '-')
    totalRevenue = stock_info.get('totalRevenue','-')
    if(totalRevenue != '-'):
        totalRevenue = toCr(totalRevenue)
    totalDebt = stock_info.get('totalDebt', '-')
    if(totalDebt != '-'):
        totalDebt = toCr(totalDebt)
    about = stock_info['longBusinessSummary']
    return jsonify({'dayLow': dayLow, 'dayHigh': dayHigh, 'yearLow': yearLow, 'yearHigh': yearHigh, 'Open': Open, 'previousClose': previousClose, 'volume': volume,
                    'currentPrice': currentPrice, 'marketCap': marketCap, 'trailingPE': trailingPE, 'forwardPE': forwardPE, 'fiftyDayAverage': fiftyDayAverage,
                    'twoHundredDayAverage': twoHundredDayAverage, 'dividendYield': dividendYield, 'totalRevenue': totalRevenue, 'totalDebt': totalDebt, 'about': about
                    })

@app.route('/logout')
def logout():
    session.pop('user_id', None)
    session.pop('username', None)
    return redirect(url_for('index'))


def toCr(num):
    num = num/10000000
    num = round(num, 2)
    num = str(num) + 'Cr'
    return num

if __name__ == '__main__':
    app.run(debug=True)

from flask import Flask, render_template, request, redirect, url_for, flash, session, jsonify
from flask_sqlalchemy import SQLAlchemy
from sqlalchemy.util import method_is_overridden
from werkzeug.security import generate_password_hash, check_password_hash
import yfinance as yf
import json

app = Flask(__name__)
app.secret_key = 'your_secret_key'  # Replace with your actual secret key

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
    stock_period = request.json['period']
    stock_interval = request.json['interval']

    tkr = yf.Ticker(stock_name)
    stk_data = tkr.history(period=stock_period, interval=stock_interval)

    # Ensure data is sorted by time
    stk_data = stk_data.sort_index()
    print(stk_data)
    # Formatting data for Chart.js financial chart
    ohlc_data = []
    for index, row in stk_data.iterrows():
        ohlc_data.append({
            'x': index.strftime('%Y-%m-%dT%H:%M:%S'),  # ISO 8601 format
            'o': row['Open'],
            'h': row['High'],
            'l': row['Low'],
            'c': row['Close']
        })

    chart_data = {
        'label': stock_name,
        'data': ohlc_data
    }
    print("reqest processed")
    return jsonify(chart_data)

@app.route('/logout')
def logout():
    session.pop('user_id', None)
    session.pop('username', None)
    return redirect(url_for('index'))

if __name__ == '__main__':
    app.run(debug=True)

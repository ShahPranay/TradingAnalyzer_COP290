document.addEventListener('DOMContentLoaded', function () {

  //Fetch available stock names on page load
  // Fetch available stock names on page load
  var stockNames = [];
  fetch('http://127.0.0.1:5000/stock_names')
    .then(response => response.json())
    .then(data => {
      stockNames = data.stock_names;
      updateStockNameDropdown();
    })
    .catch(error => {
      console.error('Fetch error:', error);
    });

  function parseData(response) {
    var ohlc = [], volume = [];

    for(let i = 0; i < response.length; i++){
      var date = Date.parse(response[i].x);
      ohlc.push([
        date,
        response[i].o,
        response[i].h,
        response[i].l,
        response[i].c,
      ]);
      volume.push([
        date,
        response[i].v
      ]);
    }
    return [{
      type: 'ohlc',
      id: 'aapl-ohlc',
      name: 'AAPL Stock Price',
      data: ohlc
    }, {
      type: 'column',
      id: 'aapl-volume',
      name: 'AAPL Volume',
      data: volume,
      yAxis: 1
    }];
  }

  document.getElementById('update').addEventListener('click', function () {
    const stockName = document.querySelector('#stock-symbol').value;
    const period = document.querySelector('#period').value;
    const interval = document.querySelector('#interval').value;

    var data_response = fetch('http://127.0.0.1:5000/data', {
      method: 'POST',
      body: JSON.stringify({
        stock_name: stockName,
        period: period,
        interval: interval
      }),
      headers: {
        'Content-Type': 'application/json'
      }
    })
      .then(response => {
        if (!response.ok) {
          throw new Error('Network response was not ok');
        }
        return response.json();
      })
      .then(data => {
        if (data && data.data) {

          myChart = Highcharts.stockChart('myChart', {
            yAxis: [{
              labels: {
                align: 'left'
              },
              height: '80%',
              resize: {
                enabled: true
              }
            }, {
              labels: {
                align: 'left'
              },
              top: '80%',
              height: '20%',
              offset: 0
            }],
            tooltip: {
              shape: 'square',
              headerShape: 'callout',
              borderWidth: 0,
              shadow: false,
              positioner: function (width, height, point) {
                const chart = this.chart;
                let position;

                if (point.isHeader) {
                  position = {
                    x: Math.max(
                      // Left side limit
                      chart.plotLeft,
                      Math.min(
                        point.plotX + chart.plotLeft - width / 2,
                        // Right side limit
                        chart.chartWidth - width - chart.marginRight
                      )
                    ),
                    y: point.plotY
                  };
                } else {
                  position = {
                    x: point.series.chart.plotLeft,
                    y: point.series.yAxis.top - chart.plotTop
                  };
                }

                return position;
              }
            },
            series: parseData(data.data),
            responsive: {
              rules: [{
                condition: {
                  maxWidth: 800
                },
                chartOptions: {
                  rangeSelector: {
                    inputEnabled: false
                  }
                }
              }]
            }
          });
        } else {
          console.error('Invalid response format:', data);
        }
      })
      .catch(error => {
        console.error('Fetch error:', error);
      });
  });

  function updateStockNameDropdown() {
    const stockNameDropdown = document.getElementById('stock-symbol');
    stockNames.forEach(name => {
      const option = document.createElement('option');
      option.value = name;
      option.text = name;
      stockNameDropdown.appendChild(option);
    });
  }

  // document.getElementById('update').addEventListener('click', update);
  document.getElementById('reset-zoom').addEventListener('click', function () {
    myChart.resetZoom();
  });
});


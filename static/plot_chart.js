var stockNames = [];

function updateSymbolsList() {
  const filter_type = document.getElementById('filterSelect').value;
  const min_filter_value = document.getElementById('filterMin').value;
  const max_filter_value = document.getElementById('filterMax').value;


  fetch('http://127.0.0.1:5000/stock_names', {
    method: 'POST',
    body: JSON.stringify({
      filter_type: filter_type,
      min_filter_value: min_filter_value,
      max_filter_value: max_filter_value,
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
      stockNames = data.stock_names;
    })
    .catch(error => {
      console.error('Fetch error:', error);
    });
}

function parseData(response, label) {
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
    id: label + '-ohlc',
    name: label + ' Stock Price',
    data: ohlc
  }, {
    type: 'column',
    id: label + '-volume',
    name: label + ' Volume',
    data: volume,
    yAxis: 1
  }];
}

function plotStockChart() {
  const stockName = document.getElementById('searchInput').value;

  var data_response = fetch('http://127.0.0.1:5000/data', {
    method: 'POST',
    body: JSON.stringify({
      stock_name: stockName,
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
          series: parseData(data.data, data.label),
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

        var today = new Date(), lastday = new Date();
        lastday.setDate(lastday.getDate() - 30);
        myChart.xAxis[0].setExtremes(lastday.getTime(), today.getTime());

      } else {
        console.error('Invalid response format:', data);
      }
    })
    .catch(error => {
      console.error('Fetch error:', error);
    });
}

function updateStockInfo (stk_info) {
  var elems = document.getElementsByClassName('box-sub-heading');
  for(let elem of elems){
    elem.innerHTML = stk_info[elem.id];
  }
  var about = document.getElementById('about');
  about.innerHTML = stk_info['about'];
}

function fetchStockInfo() {
  const stockName = document.getElementById('searchInput').value;

  var data_response = fetch('http://127.0.0.1:5000/stock_info', {
    method: 'POST',
    body: JSON.stringify({
      stock_name: stockName,
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
      updateStockInfo(data); 
    })
    .catch(error => {
      console.error('Fetch error:', error);
    });
  document.getElementById('moreInfo').style.display = 'block';
}

function toggleFilterValues() {
  var filtervals = document.getElementById('filterValues'),
    filtersel = document.getElementById('filterSelect');
  console.log(filtersel.value);
  if (filtersel.value === 'default') {
    filtervals.style.display = 'none';
  } else {
    filtervals.style.display = 'block';
  }
}

function autocomplete(input) {
  const inputValue = input.value.toLowerCase();
  const autocompleteContainer = document.getElementById('autocompleteItems');
  autocompleteItems.innerHTML = '';  

  if(inputValue === ''){
    return;
  }

  const suggestions = stockNames.filter(symb => symb.toLowerCase().startsWith(inputValue));
  suggestions.forEach(suggestion => {
    const div = document.createElement('div');
    div.textContent = suggestion;
    div.addEventListener('click', () => {
      input.value = suggestion;
      autocompleteContainer.innerHTML = '';
      plotStockChart();
      fetchStockInfo();
    });
    autocompleteContainer.appendChild(div);
  });
  console.log(autocompleteContainer.innerHTML);
}

document.addEventListener('DOMContentLoaded', function () {

  toggleFilterValues();
  updateSymbolsList();

  // Fetch available stock names selecting a filter
  document.getElementById('filterValues').addEventListener('change', function () {
    updateSymbolsList();
  });

  document.getElementById('filterSelect').addEventListener('change', function () {
    toggleFilterValues();
  });

  document.getElementById('searchInput').addEventListener('input', function() {
    autocomplete(this);
  });


});


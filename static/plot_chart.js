var stockNames = [];

// function to update the stockNames array with those that satisfy the current filters applied
function updateSymbolsList() {
  // fetch the elements that specify the type and parameters of fileter applied
  const filter_type = document.getElementById('filterSelect').value;
  const min_filter_value = document.getElementById('filterMin').value;
  const max_filter_value = document.getElementById('filterMax').value;


  // make a POST request to '/stock_names' to fetch the list of symbols satisfying the filter
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
      // update the stockNames array
      stockNames = data.stock_names;
    })
    .catch(error => {
      console.error('Fetch error:', error);
    });
}

// function to parse the stock data to a format which can be used to plot a high chart
function parseData(response, label) {
  var ohlc = [], volume = [];

  for(let i = 0; i < response.length; i++){
    // parse the date string to a timestamp
    var date = Date.parse(response[i].x);
    ohlc.push([
      date,
      response[i].o, // open
      response[i].h, // high
      response[i].l, // low
      response[i].c, // close
    ]);
    volume.push([
      date,
      response[i].v
    ]);
  }
  
  // return an array of series
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

// function to create the Stock chart using Highcharts js library
function createHighChart (series_data) {
  var retChart = Highcharts.stockChart('myChart', {
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
    series: series_data,
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

  return retChart;
}

// function to fetch stock symbol and plot the stock chart
function plotStockChart() {
  // fetch the element that specify the name of the stock that needs to be plotted
  const stockName = document.getElementById('searchInput').value;

  // make a POST request to '/data' to fetch the stock data for stockName
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
        var series_data = parseData(data.data, data.label); // call the parseData method to get the series array
        var myChart = createHighChart(series_data); // call the createHighChart method to create the HighChart

        // modify the extremes of x axis displayed to set default zoom to 1 month.
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

// function to update the stock info shown below the plot
function updateStockInfo (stk_info) {
  // fetch all the elements with the given class name
  var elems = document.getElementsByClassName('box-sub-heading');
  for(let elem of elems){
    // modify the innter HTML using  the stk_info dictionary
    elem.innerHTML = stk_info[elem.id];
  }
  // modify the about section
  var about = document.getElementById('about');
  about.innerHTML = stk_info['about'];
}

// function to fetch the stock info of the currently shown stock 
function fetchStockInfo() {
  // fetch the name of the stock being shown
  const stockName = document.getElementById('searchInput').value;

  // make a POST request to '/stock_info' to feth the required data
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
      // call the updateStockInfo method to modify the HTML tags
      updateStockInfo(data); 
    })
    .catch(error => {
      console.error('Fetch error:', error);
    });
  // make the info section visible
  document.getElementById('moreInfo').style.display = 'block';
}

// function to toggle the visiblity of the filter value input boxes depending on the filter type chosen
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

// function to display the autocomplete suggestions below the search bar
function autocomplete(input) {
  const inputValue = input.value.toLowerCase(); // convert input to lower case
  const autocompleteContainer = document.getElementById('autocompleteItems'); // fetch the autocomplete items container, we'll add our suggestions to this element
  autocompleteItems.innerHTML = '';  // reset the innter HTML

  if(inputValue === ''){
    // if the input is empty, show no suggestions
    return;
  }

  // get all the symbols for which the input is a prefix
  const suggestions = stockNames.filter(symb => symb.toLowerCase().startsWith(inputValue));

  suggestions.forEach(suggestion => {
    // create a new div element
    const div = document.createElement('div');
    div.textContent = suggestion; // suggestion is its inner text
    // things to do when this suggestion is clicked upon
    div.addEventListener('click', () => {
      input.value = suggestion; // set the input value to suggestion
      autocompleteContainer.innerHTML = ''; // clear the suggestions
      plotStockChart(); // plot the stock chart
      fetchStockInfo(); // fetch and update the stock info
    });
    // make this element a child of the autocompleteContainer 
    autocompleteContainer.appendChild(div);
  });
}

// Code to run when the document has been loaded
document.addEventListener('DOMContentLoaded', function () {

  toggleFilterValues();
  updateSymbolsList();

  // Event listener to Fetch available stock names on setting the filter values
  document.getElementById('filterValues').addEventListener('change', function () {
    updateSymbolsList();
  });
  
  // Event listener to  fetch available stock names and toggle filter value input boxes
  document.getElementById('filterSelect').addEventListener('change', function () {
    toggleFilterValues();
    updateSymbolsList();
  });

  // Event listener to draw the autocomplete suggestions whenever input in search bar is changed
  document.getElementById('searchInput').addEventListener('input', function() {
    autocomplete(this);
  });

});


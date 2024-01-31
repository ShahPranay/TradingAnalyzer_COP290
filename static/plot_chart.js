document.addEventListener('DOMContentLoaded', function () {
  const ctx = document.getElementById('myChart').getContext('2d');
  var stockNames = [];

  const zoomOptions = {
    pan: {
      enabled: true,
      mode: 'xy',
    },
    zoom: {
      wheel: {
        enabled: true
      },
      pinch: {
        enabled: true
      },
      mode: 'xy',
      // onZoomComplete({chart}) {
      //   chart.update('none');
      // }
    }
  };

  const chart_config = {
    type: 'candlestick',
    options: {
      plugins: {
        zoom: zoomOptions,
        title: {
          display: true,
          position: 'bottom',
          text: 'Zoom level: ',
        }
      }
    },
    data: {
      datasets: [{}]
    }
  };

  var myChart = new Chart(ctx, chart_config);

  function parseDates(value) {
    value.x = Date.parse(value.x);
    return value;
  }

  //Fetch available stock names on page load
  // Fetch available stock names on page load
  fetch('http://127.0.0.1:5000/stock_names')
  .then(response => response.json())
  .then(data => {
      stockNames = data.stock_names;
      updateStockNameDropdown();
  })
  .catch(error => {
      console.error('Fetch error:', error);
  });

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
          plotData = data.data.map(parseDates);
          // process plotData
          console.log(plotData);
          myChart.config.data.datasets = [
              {
                  label: data.label,
                  data: plotData
              }
          ];
          myChart.update();
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
});

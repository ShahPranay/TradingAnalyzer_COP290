document.addEventListener('DOMContentLoaded', function () {
  const ctx = document.getElementById('myChart').getContext('2d');

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

  document.getElementById('update').addEventListener('click', function () {
    const stockName = document.querySelector('#stock-symbol').value;
    const period = document.querySelector('#period').value;
    const interval = document.querySelector('#interval').value;

    fetch('http://127.0.0.1:5000/data', {
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
          var plotData = [{
            label: data.label,
            data: data.data.map(parseDates)
          }];

          myChart.config.data.datasets = plotData;

          myChart.update();

          // myChart = new Chart(ctx, {
          //     type: 'candlestick',
          //     data: {
          //         datasets: plotData
          //     }
          // });

          // Show the add symbol section
          document.getElementById('add-symbol-section').style.display = 'block';
        } else {
          console.error('Invalid response format:', data);
        }
      })
      .catch(error => {
        console.error('Fetch error:', error);
      });
  });

  // Add event listener for adding a new symbol
  document.getElementById('add-symbol').addEventListener('click', function () {
    const newSymbol = document.querySelector('#new-symbol').value;
    const period = document.querySelector('#period').value;
    const interval = document.querySelector('#interval').value;

    fetch('http://127.0.0.1:5000/data', {
      method: 'POST',
      body: JSON.stringify({
        stock_name: newSymbol,
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
          const newPlotData = data.data.map(parseDates);
          plotData.push({
            label: data.label,
            data: newPlotData
          });

          // Add new dataset to the chart
          myChart.data.datasets.push({
            label: data.label,
            data: newPlotData
          });

          // Update the chart
          myChart.update();

          // Add a new form for the added symbol
          const addedSymbolsDiv = document.getElementById('added-symbols');
          const newSymbolForm = document.createElement('form');
          newSymbolForm.innerHTML = `
                      <div class="form-group">
                          <input type="text" value="${newSymbol}" >
                      </div>
                      <div class="form-group">
                          <button class="add-symbol" data-symbol="${newSymbol}">Add</button>
                      </div>
                  `;
          addedSymbolsDiv.appendChild(newSymbolForm);

          // Clear the input for the new symbol
          document.querySelector('#new-symbol').value = '';
        } else {
          console.error('Invalid response format:', data);
        }
      })
      .catch(error => {
        console.error('Fetch error:', error);
      });
  });


});

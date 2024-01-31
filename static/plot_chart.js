document.addEventListener('DOMContentLoaded', function () {
  const ctx = document.getElementById('myChart').getContext('2d');
  var plotData = [];

  var myChart = new Chart(ctx, {
    type: 'candlestick',
    data:{
      datasets: [{
        label: 'default plot',
        data: plotData
      }]
    }
  });

  // write logic to plot default graph

  function parseDates(value) {
    value.x = Date.parse(value.x);
    return value;
  }

  document.getElementById('update').addEventListener('click', function (){
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

  // document.getElementById('update').addEventListener('click', update);
});